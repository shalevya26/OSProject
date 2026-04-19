#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifndef OSPROJECT_DIJKSTRA_H
#define OSPROJECT_DIJKSTRA_H
#define INF INT_MAX

// struct of linked list

typedef struct Node {
    int vertex;
    int weight;
    struct Node *next;
} Node;

void addEdge(Node **graph, int src, int dst, int weight);

void printPath(int parent[], int N);

int minDistance(int dist[], int visited[], int N);

void dijkstra(Node **graph,int N ,int src, int dst);

void freeGraph(Node **graph,int N);


#endif //OSPROJECT_DIJKSTRA_H