#include "Dijkstra.h"

// adds a vertex/edge to the graph
void addEdge(Node** graph, int src, int dest,int weight) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->vertex = dest;
    newNode->weight = weight;
    newNode->next = graph[src];
    graph[src] = newNode;
}

// prints the way with the parent matrix
void printPath(int parent[],int dst) {
    if (dst == -1)
        return;

    printPath(parent,parent[dst]);
    printf("%d " , dst);
}

// returns the dot with the smallest distance which wasn't visited yet
int minDistance(int dist[], int visited[], int N) {
    int min = INF;
    int minIndex = -1;

    for (int i = 0 ; i < N ; i++) {
        if (!visited[i] && dist[i] < min) {
            min = dist[i];
            minIndex = i;
        }
    }
    return minIndex;
}

void dijkstra(Node** graph,int N ,int src,int dst) {
    int* dist = (int*)malloc(sizeof(int)*N);
    int* visited = (int*)malloc(sizeof(int)*N);
    int* parent = (int*)malloc(sizeof(int)*N);

    for (int i=0;i<N;i++) {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    dist[src] = 0;

    for (int i=0;i<N-1;i++) {

        int u = minDistance(dist,visited,N);

        if (u==-1) {
            break;
        }

        visited[u] = 1;

        Node* temp = graph[u];
        while (temp != NULL) {
            int v = temp->vertex;
            int weight = temp->weight;

            if (!visited[v] && dist[u]!=INF && dist[u]+weight<dist[v]) {
                dist[v] = dist[u] + weight;
                parent[v] = u;
            }
            temp = temp->next;
        }
    }
    if (dist[dst] == INF) {
        printf("No Path from %d to %d \n",src,dst);
    }
    else {
        printf("Shortest distance from %d to %d is: %d\n",src,dst,dist[dst]);
        printf("Path: ");
        printPath(parent,dst);
        printf("\n");
    }
    free(dist);
    free(visited);
    free(parent);
}

void freeGraph(Node** graph,int N) {
    for (int i=0;i<N;i++) {
        Node* temp = graph[i];
        while (temp != NULL) {
            Node* toDelete = temp;
            temp = temp->next;
            free(toDelete);
        }
    }
    free(graph);
}

