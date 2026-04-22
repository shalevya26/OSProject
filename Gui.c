#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include "Dijkstra.h"

        /** *-------1. Calculate graphic vertex layout via BFS:-------* */

// root (0) on the left, leaves on the right
void calculateNodePositions(Node** graph, int N, Vector2* positions) {
    int depths[15]; // Max number of vertexes is 15
  // Initialize INF for each of them
    for (int i = 0; i < N; i++) depths[i] = -1;

  // Simple BFS to find depth (hop distance from root 0)
    int queue[15];
    int front = 0, rear = 0;

    queue[rear++] = 0;
    depths[0] = 0; // Root is at depth 0 (far left)

    while (front < rear) {
        int u = queue[front++];
        Node* temp = graph[u];
        while (temp) {
            int v = temp->vertex;
            if (depths[v] == -1) {
                depths[v] = depths[u] + 1;
                queue[rear++] = v;
            }
            temp = temp->next;
        }
    }

     /** *-------2.Finding needed screen proportions and spacing:-------* */

    // Find maximum depth to calculate horizontal spacing
    int max_depth = 0;
    for (int i = 0; i < N; i++) {
        if (depths[i] == -1) depths[i] = 0; // Fallback for disconnected nodes
        if (depths[i] > max_depth) max_depth = depths[i];
    }

    // Distribute X lane based on depth, Y lane based on node count at that depth
    int nodes_at_depth[15] = {0};
    int current_y_index[15] = {0};

    for (int i = 0; i < N; i++) nodes_at_depth[depths[i]]++;

    float screen_w = 1000.0f;
    float screen_h = 600.0f;

    for (int i = 0; i < N; i++) {
        int d = depths[i];

        // Calculate X coordinate (Left to Right spacing)
        float x_spacing = (screen_w - 200.0f) / (max_depth > 0 ? max_depth : 1);
        positions[i].x = 100.0f + (d * x_spacing);

        // Calculate Y coordinate (Top to Bottom spacing in the same column)
        float y_spacing = screen_h / (nodes_at_depth[d] + 1);
        current_y_index[d]++;
        positions[i].y = current_y_index[d] * y_spacing;
    }
}
          /** *-------3.Graphic Interface Initialization:-------* */

void displayGraphGUI(Node** graph, int N) {
    // Window Initialization:
    const int screenWidth = 1000;
    const int screenHeight = 600;
    //window title "Communication Network"
    InitWindow(screenWidth, screenHeight, "Communication Network");
    SetTargetFPS(60);

    Vector2 positions[15]; // Max of 15 vertices
    calculateNodePositions(graph, N, positions);

    // Main Screen Loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ---> GRAPHIC INTERFACE: Draw Edges and Weights
        // We draw edges first so they appear "under" the nodes
        for (int u = 0; u < N; u++) {
            Node* temp = graph[u];
            while (temp) {
                int v = temp->vertex;
                int weight = temp->weight;

                // Offset to the center of the rectangle (assuming rect is 40x40)
                Vector2 start = { positions[u].x + 20, positions[u].y + 20 };
                Vector2 end = { positions[v].x + 20, positions[v].y + 20 };

                // Draw the connecting line
                DrawLineEx(start, end, 2.0f, DARKGRAY);

                // Calculate arrow angle
                float angle = atan2(end.y - start.y, end.x - start.x);
                float arrow_length = 15.0f;

                // Move tip back slightly so it doesn't hide completely under the destination rectangle
                Vector2 tip = { end.x - cos(angle)*20, end.y - sin(angle)*20 };

                // Draw Arrowhead (Two lines forming a 'V')
                Vector2 p1 = { tip.x - arrow_length * cos(angle - PI/6), tip.y - arrow_length * sin(angle - PI/6) };
                Vector2 p2 = { tip.x - arrow_length * cos(angle + PI/6), tip.y - arrow_length * sin(angle + PI/6) };
                DrawLineEx(tip, p1, 2.0f, DARKGRAY);
                DrawLineEx(tip, p2, 2.0f, DARKGRAY);

                // Draw the weight text at the midpoint (small white rectangle with red text)
                int midX = (start.x + end.x) / 2;
                int midY = (start.y + end.y) / 2;
                DrawRectangle(midX - 5, midY - 10, 20, 20, RAYWHITE); // Small background to make text readable
                DrawText(TextFormat("%d", weight), midX, midY - 10, 20, RED);

                temp = temp->next;
            }
        }

        // Draw Nodes (Rectangles) and Labels based on the positions vector
        for (int i = 0; i < N; i++) {
            // Draw the computer/router node as a rectangle
            DrawRectangle(positions[i].x, positions[i].y, 40, 40, SKYBLUE);
            DrawRectangleLines(positions[i].x, positions[i].y, 40, 40, BLUE);

            // Draw the numerical identifier label in the center
            DrawText(TextFormat("%d", i), positions[i].x + 12, positions[i].y + 10, 20, BLACK);
        }

        EndDrawing();
    }

    CloseWindow();
}
