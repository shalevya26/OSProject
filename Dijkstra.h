#ifndef OSPROJECT_DIJKSTRA_H
#define OSPROJECT_DIJKSTRA_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/** * INF is defined as the maximum integer value to represent
 * nodes that are currently unreachable in the graph.
 */
#define INF INT_MAX

/**
 * Node Structure for Adjacency List
 * vertex: The destination index of the edge
 * weight: The cost/distance of the edge
 * next: Pointer to the next neighbor in the linked list
 */
typedef struct Node {
  int vertex;
  int weight;
  struct Node *next;
} Node;

/**
 * Adds a new directed edge to the adjacency list.
 * @param graph: Pointer to the array of linked list heads
 * @param src: The starting vertex index
 * @param dst: The destination vertex index
 * @param weight: The weight of the edge
 */
void addEdge(Node **graph, int src, int dst, int weight);

/**
 * Prints the reconstructed path from source to destination.
 * @param parent: Array storing the predecessor of each node
 * @param dst: The destination vertex index
 * @param is_last_node: Boolean-style flag (1 if last node, 0 otherwise) for
 * formatting
 */
void printPath(int parent[], int N, int last);

/**
 * Finds the unvisited vertex with the minimum distance value.
 * @param dist: Current shortest distance array
 * @param visited: Boolean array of visited nodes
 * @param N: Total number of vertices
 * @return The index of the nearest unvisited vertex
 */
int minDistance(int dist[], int visited[], int N);

/**
 * Executes Dijkstra's algorithm to find the shortest path between two nodes.
 * @param graph: The adjacency list representation
 * @param N: Total number of vertices
 * @param src: The starting vertex
 * @param dst: The target vertex
 */
void dijkstra(Node **graph, int N, int src, int dst);

/**
 * Deep-frees the graph memory, including all linked list nodes.
 * @param graph: The adjacency list to free
 * @param N: Number of vertex head pointers in the array
 */
void freeGraph(Node **graph, int N);

#endif  // OSPROJECT_DIJKSTRA_H