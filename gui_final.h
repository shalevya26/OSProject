#include "Dijkstra.h"
#include <stdbool.h>
#include <semaphore.h>

#ifndef OSPROJECT_GUI_FINAL_H
#define OSPROJECT_GUI_FINAL_H

typedef enum {
  ACTION_UPDATE_GUI,
  ACTION_REQ_NODE,
  ACTION_RELEASE_NODE
} ActionType;

typedef struct {
  pid_t pid;
  int traveler_idx;
  ActionType action;
  int node_id;
  int intended_next_node; // Used for console logs
  int job_len;            // Used for SJF priority
  bool is_destination;
} TravelerMsg;

void displayGraphGUI_M6(Node** graph, int N, int read_fd, int num_travelers, const char* algo, sem_t** go_sems);


#endif  // OSPROJECT_GUI_FINAL_H
