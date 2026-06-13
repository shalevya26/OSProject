#ifndef GUI_H
#define GUI_H

#include "Dijkstra.h"

// Added path parameters (Milestone 3)
void displayGraphGUI(Node** graph, int N, int* path, int path_len);

#ifdef MILESTONE4
#include <sys/types.h>
// Added array parameters for multiple parallel travelers (Milestone 4)
void displayGraphGUI_M4(Node** graph, int N, int** paths, int* path_lens,
                        pid_t* pids, int num_travelers);
#endif

#ifdef MILESTONE5
void displayGraphGUI_M5(Node** graph, int N, int read_fd, int num_travelers);
#endif

#ifdef MILESTONE6
void displayGraphGUI_M6(Node** graph, int N,int read_fd, int num_travelers);
#endif


#endif