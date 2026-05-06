#include <stdio.h>
#include <stdlib.h>
#include "Dijkstra.h"


#ifdef ENABLE_GUI
#include "Gui.h"
#endif

int main(int argc, char *argv[]) {

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
  int N, M;  // N = Number of vertices, M = Number of edges
  if (fscanf(file, "%d %d", &N, &M) != 2) {
    printf("Failed to read graph dimensions\n");
    fclose(file);
    return 1;
  }

  if (N < 0 || M < 0) {
    printf("Invalid graph variables: N and M must be non-negative\n");
    fclose(file);
    return 1;
  }

  /* --- 3. Memory Allocation --- */
  // Allocate an array of pointers to Node (Adjacency List)
  Node** graph = (Node**)malloc(sizeof(Node*) * N);
  if (!graph) {
    printf("Memory allocation failed for graph head pointers\n");
    fclose(file);
    exit(1);
  }

  // Initialize all head pointers to NULL to avoid garbage values
  for (int i = 0; i < N; i++) {
    graph[i] = NULL;
  }

  /* --- 4. Building the Adjacency List --- */
  for (int i = 0; i < M; i++) {
    int src, dst, weight;
    if (fscanf(file, "%d %d %d", &src, &dst, &weight) == 3) {
      // Basic bounds checking for vertex indices
      if (src < 0 || src >= N || dst < 0 || dst >= N || weight < 0) {
        printf("Invalid edge detected at line %d\n", i + 2);
        continue;
      }
      addEdge(graph, src, dst, weight);
    }
  }

  /* --- 5. Path Query and Algorithm Execution --- */
  int src, dst;
  if (fscanf(file, "%d %d", &src, &dst) == 2) {
    fclose(file);  // Close file as soon as we're done reading

    //Added array variables to capture the path from dijkstra
    int* path = NULL;
    int path_len = 0;

    // Run Dijkstra's Algorithm to find the shortest path (runs for both milestones)
    dijkstra(graph, N, src, dst, &path, &path_len);


#ifdef ENABLE_GUI
    // ---- Launch the Raylib GUI ----
    // ---> MILESTONE 3 ADDITION: pass path to GUI
    displayGraphGUI(graph, N, path, path_len);
#endif

    //Free dynamically allocated path
    if (path) free(path);

  } else {
    printf("Could not read source and destination nodes\n");
    fclose(file);
  }

  /* --- 6. Cleanup --- */
  // Free all linked lists and the primary pointer array
  freeGraph(graph, N);

  return 0;
}
