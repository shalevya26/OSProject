#include "Dijkstra.h"

/**
 * Adds a directed edge to the adjacency list. New nodes are inserted at the
 * head of the list.
 */
void addEdge(Node** graph, int src, int dest, int weight) {
  Node* newNode = (Node*)malloc(sizeof(Node));
  if (!newNode) {
    printf("Error: Could not allocate memory for new edge node\n");
    return;
  }

  newNode->vertex = dest;
  newNode->weight = weight;

  // Linked list head insertion
  newNode->next = graph[src];
  graph[src] = newNode;
}

/**
 * Recursively prints the path from source to destination using the parent
 * array.
 * @param last: boolean-style flag to prevent printing the trailing arrow '->'
 */
void printPath(int parent[], int dst, int last) {
  // Base case: we've reached the start node's parent (-1)
  if (dst == -1) return;

  // Recurse first to print path in correct order (src -> ... -> dst)
  printPath(parent, parent[dst], 0);

  printf("%d", dst);
  if (!last) {
    printf(" -> ");
  }
}

/**
 * Helper: Finds the unvisited vertex with the minimum distance.
 */
int minDistance(int dist[], int visited[], int N) {
  int min = INF;
  int minIndex = -1;

  for (int i = 0; i < N; i++) {
    if (!visited[i] && dist[i] < min) {
      min = dist[i];
      minIndex = i;
    }
  }
  return minIndex;
}

/**
 * Main Dijkstra's Algorithm implementation.
 * Finds the shortest path and prints the result.
 */
void dijkstra(Node** graph, int N, int src, int dst) {
  int* dist = (int*)malloc(sizeof(int) * N);
  int* visited = (int*)malloc(sizeof(int) * N);
  int* parent = (int*)malloc(sizeof(int) * N);
  if (!dist || !visited || !parent) {
    printf("Algorithm memory allocation failed\n");
    exit(1);
  }

  // Initialization phase
  for (int i = 0; i < N; i++) {
    dist[i] = INF;
    visited[i] = 0;
    parent[i] = -1;
  }

  dist[src] = 0;
  // Core Loop: Process N-1 vertices
  for (int i = 0; i < N - 1; i++) {
    int u = minDistance(dist, visited, N);

    // If no reachable unvisited nodes remain, stop
    if (u == -1) {
      break;
    }

    visited[u] = 1;

    // Relax edges: Update neighbors of the current node u
    Node* temp = graph[u];
    while (temp != NULL) {
      int v = temp->vertex;
      int weight = temp->weight;
     // Relaxation step
      if (!visited[v] && dist[u] != INF && dist[u] + weight < dist[v]) {
        dist[v] = dist[u] + weight;
        parent[v] = u;
      }
      temp = temp->next;
    }
  }

  /* Output Results */
  if (dist[dst] == INF) {
    printf("No path found\n");
  } else {
    printPath(parent, dst, 1);
    printf("\n");
    printf("%d\n", dist[dst]);
  }

  // Memory Cleanup (Algorithm internals only)
  free(dist);
  free(visited);
  free(parent);
}

/**
 * Deep-frees the graph: every node in the adjacency lists and the array itself.
 */
void freeGraph(Node** graph, int N) {
  if (!graph) return;

  for (int i = 0; i < N; i++) {
    Node* temp = graph[i];
    while (temp != NULL) {
      Node* toDelete = temp;
      temp = temp->next;
      free(toDelete);
    }
  }
  free(graph);
}
