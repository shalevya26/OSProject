#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Dijkstra.h"
#include "gui_final.h"
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

typedef struct {
  pid_t pid;
  int current_node;
  int next_node;
  bool is_destination;
} TravelerMsg;

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <file_name>\n", argv[0]);
    return 1;
  }

  /* --- 1. File Handling --- */
  FILE* file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("Error opening %s\n", argv[1]);
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

  sem_t** node_sems = malloc(N * sizeof(sem_t*));
  for (int i = 0; i < N; i++) {
    char sem_name[64];
    sprintf(sem_name, "/node_sem_%d_%d", getpid(), i);
    sem_unlink(sem_name);
    node_sems[i] = sem_open(sem_name, O_CREAT, 0666, 1);
    if (node_sems[i] == SEM_FAILED) {
      perror("sem_open failed");
      exit(EXIT_FAILURE);
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
        printf("[%d] started\n", getpid());

        // 2. Travel the path and report to the parent
        if (path_len > 0) {
          sem_wait(node_sems[path[0]]);
        }


        for (int j = 0; j < path_len; j++) {
          msg.current_node = path[j];

          if (j < path_len - 1) {
            msg.next_node = path[j + 1];
            msg.is_destination = false;
          } else {
            msg.next_node = -1;
            msg.is_destination = true;
          }

          write(pipefd[1], &msg, sizeof(TravelerMsg));
          sleep(1);
          if (j < path_len - 1) {
            sem_wait(node_sems[path[j + 1]]);
            sem_post(node_sems[path[j]]);
          }
        }

        if (path_len > 0) {
          sem_post(node_sems[path[path_len - 1]]);
        }


        // 3. Child Cleanup: Free memory inherited from the parent before
        // exiting to ensure a perfectly clean Valgrind report
        if (path) free(path);
        close(pipefd[1]);  // Close write-end when finished
        freeGraph(graph, N);
        fclose(file);
        free(pids);

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

  displayGraphGUI_M6(graph, N, pipefd[0], num_travelers);


  for (int i = 0; i < num_travelers; i++) {
    waitpid(pids[i], NULL, 0);
  }

  close(pipefd[0]);
  free(pids);
  for (int i = 0; i < N; i++) {
    char sem_name[64];

    sprintf(sem_name, "/node_sem_%d_%d", getpid(), i);

    sem_close(node_sems[i]);
    sem_unlink(sem_name);
  }

  free(node_sems);


  /* --- 6. Cleanup --- */
  freeGraph(graph, N);
  return 0;
}