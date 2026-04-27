#include <raylib.h>
#include <math.h>
#include <stdio.h>
#include "Dijkstra.h"

/** *-------1. Calculate graphic vertex layout:-------* */

void calculateNodePositions(Node** graph, int N, Vector2* positions) {
    // Circular layout algorithm.
    // Evenly distributes nodes around a center point based on the total number of nodes (N).
    // Screen proportions for calculation:
    float screen_w = 1000.0f;
    float screen_h = 600.0f;
    float center_x = screen_w / 2.0f;
    float center_y = screen_h / 2.0f;
    float radius = 220.0f;

     // Creating a "circle" of nodes:
    for (int i = 0; i < N; i++) {
        // Calculate angle for each node. 2 * PI is a full circle.
        float angle = (2.0f * PI * i) / N;

        // X = center_x + radius * cos(angle)
        // Y = center_y + radius * sin(angle)
        // We subtract 20 to center the 40x40 rectangles properly.
        positions[i].x = center_x + radius * cos(angle) - 20;
        positions[i].y = center_y + radius * sin(angle) - 20;
    }
}

/** *-------2.Graphic Interface Initialization:-------* */

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

        //Draw Edges and Weights
        //Edges first so they appear "under" the nodes
        for (int u = 0; u < N; u++) {
            Node* temp = graph[u];
            while (temp) {
                int v = temp->vertex;
                int weight = temp->weight;

                // Offset to the center of the rectangle (assuming rect is 40x40)
                Vector2 start = { positions[u].x + 20, positions[u].y + 20 };
                Vector2 end = { positions[v].x + 20, positions[v].y + 20 };

                // Calculate edge angle
                float angle = atan2(end.y - start.y, end.x - start.x);
                float nx = -sin(angle);
                float ny = cos(angle);
                float offset = 10.0f; // Shift the line 10 pixels orthogonally so returning edges won't overlap

                // Apply the offset to both start and end points
                start.x += nx * offset;
                start.y += ny * offset;
                end.x += nx * offset;
                end.y += ny * offset;

                // Draw the connecting line using the shifted coordinates
                DrawLineEx(start, end, 2.0f, DARKGRAY);

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