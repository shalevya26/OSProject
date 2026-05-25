#include <stdio.h>
#include <stdlib.h>

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

#else
  // ---> ORIGINAL MILESTONE 1-3 LOGIC (Single Traveler)
  int src, dst;
  if (fscanf(file, "%d %d", &src, &dst) == 2) {
    fclose(file);

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