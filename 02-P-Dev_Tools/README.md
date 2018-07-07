##  02-P-Dev_Tools

* __lee.c__ - Lee algorithm
* __queue.c__ - Queue structure implementation for BFS
* __mazeSearch.c__ - Pass random maze of size m * n for running time calculation of Lee algorithm
* __Makefile__ - use ```make CFLAGS=Oi``` where ```i = [0,1,2,3,s]``` for different optimization options

*lee.c uses [Lee algorithm](https://en.wikipedia.org/wiki/Lee_algorithm) to find the shortest path in a maze from point A to point B. It searches for a shortest-path connection between the source and destination nodes of a connection by performing a breadth-first search and labeling each grid point with its distance from the source. This expansion phase will eventually reach the destination node __if__ a connection __is possible__. A second trace back phase then forms the connection by following any path with decreasing labels. This algorithm is guaranteed to find the shortest path between a source and destination for a given connection.*
