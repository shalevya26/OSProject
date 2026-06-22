#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "Dijkstra.h"
#include "gui_final.h"

int calculateRemainingWork(Node** graph, int* path, int path_len, int start_index) {
  int work = 0;

  for (int i = start_index; i < path_len - 1; i++) {
    Node* temp = graph[path[i]];

    while (temp) {
      if (temp->vertex == path[i + 1]) {
        work += temp->weight;
        break;
      }
      temp = temp->next;
    }
  }

  return work;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("Usage: %s -schd <fcfs|sjf> <file_name>\n", argv[0]);
    return 1;
  }
  if (strcmp(argv[1], "-schd") != 0) {
    printf("Error: Missing -schd flag. Usage: %s -schd <fcfs|sjf> <file_name>\n", argv[0]);
    return 1;
  }

  char* algo = argv[2];
  if (strcmp(algo, "fcfs") != 0 && strcmp(algo, "sjf") != 0) {
    printf("Error: Scheduler algorithm must be 'fcfs' or 'sjf'\n");
    return 1;
  }


  /* --- 1. File Handling --- */
  FILE* file = fopen(argv[3], "r");
  if (file == NULL) {
    printf("Error opening %s\n", argv[3]);
    return 1;
  }

  /* --- 2. Graph Metadata Input --- */
  int N = -1, M = -1;  // N = Number of vertices, M = Number of edges
  char line[256];

  while (fscanf(file, "%s", line) == 1) {
    if (line[0] == '#') {
      fgets(line, sizeof(line), file);  // Skip the rest of the comment line
    } else {
      N = atoi(line);          // The first non-comment string is N
      fscanf(file, "%d", &M);  // The very next integer is M
      break;                   // Break out of the loop
    }
  }

  if (N < 0 || M < 0) {
    printf("Invalid graph variables: N and M must be non-negative\n");
    fclose(file);
    return 1;
  }

  /* --- 3. Memory Allocation --- */
  Node** graph = (Node**)malloc(sizeof(Node*) * N);
  if (!graph) {
    printf("Memory allocation failed for graph head pointers\n");
    fclose(file);
    exit(1);
  }

  for (int i = 0; i < N; i++) {
    graph[i] = NULL;
  }

  /* --- 4. Building the Adjacency List --- */
  for (int i = 0; i < M; i++) {
    int src, dst, weight;
    if (fscanf(file, "%d %d %d", &src, &dst, &weight) == 3) {
      if (src < 0 || src >= N || dst < 0 || dst >= N || weight < 0) {
        printf("Invalid edge detected at line %d\n", i + 2);
        continue;
      }
      addEdge(graph, src, dst, weight);
    }
  }


  int num_travelers = 0;

  // 1. Find the number of travelers
  while (fscanf(file, "%s", line) == 1) {
    if (line[0] == '#') {
      fgets(line, sizeof(line), file);
    } else {
      num_travelers = atoi(line);
      break;
    }
  }

  // 2. Setup IPC using pipe
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe failed");
    fclose(file);
    freeGraph(graph, N);
    exit(EXIT_FAILURE);
  }


  // Allocate array for parent to track PIDs
  pid_t* pids = malloc(num_travelers * sizeof(pid_t));
  if (!pids) {
    printf("Memory allocation failed for PIDs\n");
    fclose(file);
    freeGraph(graph, N);
    exit(EXIT_FAILURE);
  }


  // Create Parent-managed Wake-up Semaphores (One per traveler)
  sem_t** go_sems = malloc(num_travelers * sizeof(sem_t*));
  for (int i = 0; i < num_travelers; i++) {
    char sem_name[64];
    sprintf(sem_name, "/go_sem_%d_%d", getpid(), i);
    sem_unlink(sem_name);
    go_sems[i] = sem_open(sem_name, O_CREAT, 0666, 0); // Start locked (0)
  }

  // 3. Loop and fork children
  for (int i = 0; i < num_travelers; i++) {
    int src, dst;
    if (fscanf(file, "%d %d", &src, &dst) == 2) {
      pid_t pid = fork();

      if (pid < 0) {
        perror("fork failed");
      } else if (pid == 0) {
        // ==========================================
        //             CHILD PROCESS
        // ==========================================

        close(pipefd[0]);  // Child doesn't read from the pipe, close read-end

        int* path = NULL;
        int path_len = 0;

        // 1. Child calculates its OWN route autonomously [cite: 52]
        dijkstra(graph, N, src, dst, &path, &path_len);

        TravelerMsg msg;
        msg.pid = getpid();
        msg.traveler_idx = i;
        printf("[%d] started\n", getpid());

        // 2. Travel the path and report to the parent
        if (path_len > 0) {
          // Request starting node
          msg.action = ACTION_REQ_NODE;
          msg.node_id = path[0];
          msg.job_len = calculateRemainingWork(graph, path, path_len, 0); // SJF priority (total work)
          write(pipefd[1], &msg, sizeof(TravelerMsg));
          sem_wait(go_sems[i]);   // Block until Parent grants access
        }


        for (int j = 0; j < path_len; j++) {
          // Tell GUI we entered the node
          msg.action = ACTION_UPDATE_GUI;
          msg.node_id = path[j];
          msg.intended_next_node = (j < path_len - 1) ? path[j + 1] : -1;
          msg.is_destination = (j == path_len - 1);
          write(pipefd[1], &msg, sizeof(TravelerMsg));

          sleep(1); // Simulate travel time

          if (j < path_len - 1) {
            // Request Next Node
            msg.action = ACTION_REQ_NODE;
            msg.node_id = path[j + 1];
            msg.job_len = calculateRemainingWork(graph, path, path_len, j); // SJF priority (remaining path)
            write(pipefd[1], &msg, sizeof(TravelerMsg));
            sem_wait(go_sems[i]);

            // Release Previous Node (Only after acquiring the next!)
            msg.action = ACTION_RELEASE_NODE;
            msg.node_id = path[j];
            write(pipefd[1], &msg, sizeof(TravelerMsg));
          }
        }

        if (path_len > 0) {
          msg.action = ACTION_RELEASE_NODE;
          msg.node_id = path[path_len - 1];
          write(pipefd[1], &msg, sizeof(TravelerMsg));
        }


        // 3. Child Cleanup: Free memory inherited from the parent before
        // exiting to ensure a perfectly clean Valgrind report
        if (path) free(path);
        close(pipefd[1]);  // Close write-end when finished
        freeGraph(graph, N);
        fclose(file);
        free(pids);
        for (int k = 0; k < num_travelers; k++) sem_close(go_sems[k]);
        free(go_sems);


        exit(EXIT_SUCCESS);
      } else {
        // ==========================================
        //             PARENT PROCESS
        // ==========================================
        pids[i] = pid;  // Save child's PID
      }
    }
  }

  close(pipefd[1]);
  fclose(file);

  displayGraphGUI_M6(graph, N, pipefd[0], num_travelers, algo, go_sems);


  for (int i = 0; i < num_travelers; i++) {
    waitpid(pids[i], NULL, 0);
  }

  close(pipefd[0]);
  free(pids);

  for (int i = 0; i < num_travelers; i++) {
    char sem_name[64];
    sprintf(sem_name, "/go_sem_%d_%d", getpid(), i);
    sem_close(go_sems[i]);
    sem_unlink(sem_name);
  }
  free(go_sems);

  /* --- 6. Cleanup --- */
  freeGraph(graph, N);
  return 0;
}