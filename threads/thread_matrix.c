#define _GNU_SOURCE

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>

static bool is_verbose = false;

#ifdef DEBUG
static inline int plog(const char *format, ...)
{
	if (!is_verbose)
		return 0;
	va_list arg;
	va_start(arg, format);
	int ret = fprintf(stderr, "[LOG]: ");
	ret += vfprintf(stderr, format, arg);
	va_end(arg);
	return ret;
}
#else
#define plog(...)
#endif

static const char help_str[] = {
	"[-h] [-v] -t NUM_THREADS -c NUM_CORES -W Width -H Height\n"
	"Calculate the time it takes to multiply 2 matrixes"
	" H*W and W*H filled with random numbers"
};

/**
 * timespec_diff() - returns time difference in milliseconds for two timespecs.
 * @stop:	time after event.
 * @start:	time before event.
 *
 * Uses difftime() for time_t seconds calcultation.
 */
static double timespec_diff(struct timespec *stop, struct timespec *start)
{
	double diff = difftime(stop->tv_sec, start->tv_sec);
	diff *= 1000;
	diff += (stop->tv_nsec - start->tv_nsec) / 1e6;
	return diff;
}

enum _errors {
	E_OK = 0,
	E_FOPEN,
	E_FREAD,
	E_ALLOC,
	E_CPUSET
};

static const char * const _error_msg[] = {
	[E_OK] = "Success",
	[E_FOPEN] = "Failed to open '/dev/random'",
	[E_FREAD] = "Failed to read from '/dev/random'",
	[E_ALLOC] = "Failed to allocate memory",
	[E_CPUSET] = "Could not link thread to all CPU cores"
};

/**
 * struct thread_data - data that is passed to each thread
 * that multiply matrixes.
 *
 * @matr1:	pointer to the start of matrix1 fragment
 * @matr2:	pointer to whole matrix2
 * @res:	pointer to result matrix
 * @height1:	number of rows in matrix1
 * @width1:	width of matrix1
 * @width2:	width of matrix2
 *
 * height2 is not included because it must be equal to @width1.
 */
struct thread_data {
	struct timespec start_time, end_time;
	int **matr1, **matr2, **res;
	int height1, width1, width2;
};


void *threadfunc(void *args)
{
	struct thread_data *data = args;

	clock_gettime(CLOCK_REALTIME, &data->start_time);

	for (int i = 0; i < data->height1; i++)
		for (int j = 0; j < data->width2; j++) {
			int tmp = 0;
			for (int k = 0; k < data->width1; k++)
				tmp += data->matr1[i][k] * data->matr2[k][j];
			data->res[i][j] = tmp;
		}

	clock_gettime(CLOCK_REALTIME, &data->end_time);

	return 0;
}

/**
 * set_cores() - set number of cores a thread is elligible
 * to run on.
 *
 * @cores_on:	if not 0, designates the number of cores,
 * 		accessible by a thread
 */
static cpu_set_t set_cores(int cores_on)
{
	cpu_set_t cpuset;
	pthread_t this = pthread_self();
	pthread_getaffinity_np(this, sizeof cpuset, &cpuset);
	int numcores = sysconf(_SC_NPROCESSORS_ONLN);
	if (cores_on <= 0)
		cores_on = numcores;
	for (int id = 0; id < numcores; id++) {
		plog("Core %d was %d\n",id, CPU_ISSET(id, &cpuset));
		if (cores_on-- > 0)
			CPU_SET(id, &cpuset);
		else
			CPU_CLR(id, &cpuset);
		plog("Core %d is %d\n",id, CPU_ISSET(id, &cpuset));
	}
	pthread_setaffinity_np(this, sizeof cpuset, &cpuset);
	return cpuset;
}

static enum _errors getrandmatrix_m(int ***mat, int h, int w)
{
	*mat = malloc(h * sizeof *mat);

	if (NULL == *mat)
		return  E_ALLOC;
	for (int i = 0; i < h; i++) {
		if (NULL == ((*mat)[i] = malloc(w * sizeof *mat)))
			return E_ALLOC;
		for (int j = 0; j < w; j++)
			(*mat)[i][j] = rand() / (RAND_MAX/100);
	}

	return E_OK;
}

static inline void freematrix(int ***mat, int h)
{
	while (h-- > 0)
		free((*mat)[h]);
	free(*mat);
}

int main(int argc, char *argv[])
{
	int num_threads = 0;
	int width = 0, height = 0;
	int num_cores = 0;

	plog("Arguments given:\n");
	for (int i = 0; i < argc; i++)
		plog("\t[%d]: '%s'\n", i, argv[i]);

	opterr = 0;
	int argopt;
	while ((argopt = getopt(argc, argv, "hvc:t:W:H:")) != -1) {
		switch (argopt) {
		case 'h':
			printf("Usage: %s %s\n", argv[0], help_str);
			exit(0);
		case 'v':
			is_verbose = true;
			break;
		case 'c':
			num_cores = atoi(optarg);
			break;
		case 't':
			num_threads = atoi(optarg);
			break;
		case 'W':
			width = atoi(optarg);
			break;
		case 'H':
			height = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Unknown option '%s'\n", optarg);
			exit(EXIT_FAILURE);
		}
	}
	plog("Parsed arguments:\n"
	     "\tis_verbose\t=%s\n\tnum_threads\t=%d\n\tW\t=%d\n\tH\t=%d\n\tnum_cores\t=%d\n",
	     (is_verbose ? "true" : "false"), num_threads, width, height, num_cores);
	if (argc <= 1) {
		printf("Usage: %s %s\n", argv[0], help_str);
		exit(0);
	}
	if (num_threads <= 0 || width <= 0 || height <= 0) {
		fprintf(stderr, "NUM_THREADS, Width and Height must be ints > 0\n");
		exit(EXIT_FAILURE);
	}

	pthread_t threads[num_threads];
	struct thread_data th_data[num_threads];

	enum _errors err = E_OK;
	FILE *fp_rand = fopen("/dev/random", "rb");
	if (NULL == fp_rand) {
		err = E_FOPEN;
		goto exc_fopen;
	}

	unsigned int seed;
	fread(&seed, sizeof(seed), 1, fp_rand);
	if (ferror(fp_rand)) {
		err = E_FREAD;
		goto exc_fread;
	}
	srand(seed);

	int **m1, **m2, **res;
	if (E_OK != (err = getrandmatrix_m(&m1, height, width)) ||
	    E_OK != (err = getrandmatrix_m(&m2, width, height)) ||
	    E_OK != (err = getrandmatrix_m(&res, height, height))) {
		err = E_ALLOC;
		goto exc_alloc;
	}

	/* Configure thread attributes */
	pthread_attr_t thread_attrs;
	pthread_attr_init(&thread_attrs);

	pthread_attr_setschedpolicy(&thread_attrs, SCHED_FIFO);
	pthread_setschedprio(pthread_self(), sched_get_priority_max(SCHED_FIFO));
	struct sched_param param;
	pthread_attr_getschedparam(&thread_attrs, &param);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_attr_setschedparam(&thread_attrs, &param);

	cpu_set_t cpuset = set_cores(num_cores);
	if (pthread_attr_setaffinity_np(&thread_attrs, sizeof cpuset, &cpuset) < 0) {
		err = E_CPUSET;
		goto exc_aff;
	}

	struct timespec time_now, time_after;
	clock_gettime(CLOCK_REALTIME, &time_now);
	int height_slice = height / num_threads;
	int rest = height % num_threads; /* allows for any value of height */

	for(int i = 0; i < num_threads; i++) {
		th_data[i].matr1 = &m1[i * height_slice];
		th_data[i].matr2 = m2;
		th_data[i].res = &res[i * height_slice];

		height_slice += (i == num_threads - 1) ? rest : 0;
		th_data[i].height1 = height_slice;
		th_data[i].width1 = width;
		th_data[i].width2 = height;

		pthread_create(&threads[i], &thread_attrs,
			       &threadfunc, &th_data[i]);
	}

	plog("Threads created. Performing join..\n");
	for (int i = 0; i < num_threads; i++)
		pthread_join(threads[i], NULL);

	clock_gettime(CLOCK_REALTIME, &time_after);

	double took_global = timespec_diff(&time_after, &time_now);
	double took_avg = 0.;
	for (int i = 0; i < num_threads; i++)
		took_avg += timespec_diff(&(th_data[i].end_time),
					  &(th_data[i].start_time));
	took_avg /= num_threads;

	printf("Height: %d\nWidth: %d\nThreads: %d\n\n"
		"Average thread time, ms: %g\nCalculation took, ms: %g\n",
		height, width, num_threads, took_avg, took_global);

	freematrix(&m1, height);
	freematrix(&m2, width);
	freematrix(&res, height);

	exc_aff:
		pthread_attr_destroy(&thread_attrs);
	exc_fread:
		fclose(fp_rand);
	exc_fopen:
	exc_alloc:

	if (E_OK == err)
		return 0;

	fprintf(stderr, "Error: %s\n",_error_msg[err]);
	exit(EXIT_FAILURE);
}
