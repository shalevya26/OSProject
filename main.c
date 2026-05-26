#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Dijkstra.h"

#ifdef ENABLE_GUI
#include "Gui.h"
#endif

// ---> MILESTONE 4 ADDITION: OS libraries for fork, wait, and signals
#ifdef MILESTONE4
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef MILESTONE5
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Message struct from son to father
typedef struct {
  pid_t pid;
  int current_node;
  int next_node;
  bool is_destination;
} TravelerMsg;
#endif

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

  /* --- 5. Path Query and Algorithm Execution --- */

#ifdef MILESTONE4
  // ---> MILESTONE 4 LOGIC: Multiple Travelers and Multiprocessing
  int num_travelers = 0;

  // 1. Now look for the number of son travelers (skipping optional comments
  // like "#travelers")
  while (fscanf(file, "%s", line) == 1) {
    if (line[0] == '#') {
      fgets(line, sizeof(line), file);  // skip the rest of the text line
    } else {
      num_travelers = atoi(line);
      break;
    }
  }

  // 2. Allocate memory and process the sons
  pid_t* pids = malloc(num_travelers * sizeof(pid_t));
  int** paths = malloc(num_travelers * sizeof(int*));
  int* path_lens = malloc(num_travelers * sizeof(int));

  for (int i = 0; i < num_travelers; i++) {
    int src, dst;
    if (fscanf(file, "%d %d", &src, &dst) == 2) {
      if (src < 0 || src >= N || dst < 0 || dst >= N) {
        printf("Invalid route\n");
        exit(1);
      }
      // Parent calculates the path FIRST
      dijkstra(graph, N, src, dst, &paths[i], &path_lens[i]);

      // Fork the child process
      pid_t pid = fork();

      if (pid < 0) {
        printf("Fork failed!\n");
      } else if (pid == 0) {
        // --- CHILD PROCESS ---
        printf("[%d] started\n", getpid());

        // Child goes to sleep. It will be killed by the parent later.
        while (1) {
          sleep(1);
        }
        exit(0);
      } else {
        // --- PARENT PROCESS ---
        pids[i] = pid;  // Save child's PID to track it
      }
    }
  }
  fclose(file);

#ifdef ENABLE_GUI
  // Parent runs the new Milestone 4 GUI, passing arrays of paths and PIDs
  displayGraphGUI_M4(graph, N, paths, path_lens, pids, num_travelers);
#endif

  // Cleanup: Ensure all children are killed and waited for
  for (int i = 0; i < num_travelers; i++) {
    kill(pids[i], SIGKILL);     // Failsafe kill
    waitpid(pids[i], NULL, 0);  // Wait to prevent zombie processes
    if (paths[i]) free(paths[i]);
  }
  free(paths);
  free(path_lens);
  free(pids);

#endif

#ifdef MILESTONE5
  // ---> MILESTONE 5 LOGIC: Autonomous Children & IPC

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
        for (int j = 0; j < path_len; j++) {
          msg.current_node = path[j];

          if (j < path_len - 1) {
            msg.next_node = path[j + 1];
            msg.is_destination = false;
          } else {
            msg.next_node = -1;  // No next node
            msg.is_destination = true;
          }

          // Send current state to the parent pipe [cite: 52]
          write(pipefd[1], &msg, sizeof(TravelerMsg));

          // Sleep to simulate travel time and let the GUI update smoothly
          sleep(1);
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

#ifdef ENABLE_GUI
  // 4. Parent runs the GUI, passing the read-end of the pipe
  displayGraphGUI_M5(graph, N, pipefd[0], num_travelers);
#endif

  for (int i = 0; i < num_travelers; i++) {
    waitpid(pids[i], NULL, 0);
  }

  close(pipefd[0]);
  free(pids);

#else
  // ---> ORIGINAL MILESTONE 1-3 LOGIC (Single Traveler)
  int src, dst;
  if (fscanf(file, "%d %d", &src, &dst) == 2) {
    fclose(file);

    if (src < 0 || src >= N || dst < 0 || dst >= N) {
      printf("Invalid route\n");
      freeGraph(graph, N);
      exit(1);
    }

    int* path = NULL;
    int path_len = 0;

    dijkstra(graph, N, src, dst, &path, &path_len);

#ifdef ENABLE_GUI
#ifdef MILESTONE3
    displayGraphGUI(graph, N, path, path_len);
#else
    displayGraphGUI(graph, N, NULL, 0);
#endif
#endif

    if (path) free(path);
  } else {
    printf("Could not read source and destination nodes\n");
    fclose(file);
  }
#endif

  /* --- 6. Cleanup --- */
  freeGraph(graph, N);
  return 0;
}