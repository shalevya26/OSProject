#include <stdio.h>
#include "Dijkstra.h"

int main(void) {

    FILE* file = fopen("graph.txt", "r");
    if (file == NULL) {
        printf("Unable to open file");
        return 1;
    }

    int N,M;
    fscanf(file, "%d %d", &N, &M);

    Node** graph = (Node**)malloc(sizeof(Node*) * N);
    for (int i = 0; i < M; i++) {
        int src, dst, weight;
        fscanf(file, "%d %d %d", &src, &dst, &weight);
        addEdge(graph,src,dst,weight);
    }

    int src,dst;
    fscanf(file, "%d %d", &src, &dst);

    fclose(file);

    dijkstra(graph,N,src,dst);

    freeGraph(graph,N);

    return 0;
}