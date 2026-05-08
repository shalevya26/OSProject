#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "Dijkstra.h"

/** *-------1. Calculate graphic vertex layout:-------* */

void calculateNodePositions(int N, Vector2* positions) {
  // Circular layout algorithm.
  // Evenly distributes nodes around a center point based on the total number of
  // nodes (N). Screen proportions for calculation:
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

// Added path parameters
void displayGraphGUI(Node** graph, int N, int* path, int path_len) {
#ifndef MILESTONE3
  // Silence compiler warnings for M2 since it doesn't use the path variables
  (void)path;
  (void)path_len;
#endif

  // Window Initialization:
  const int screenWidth = 1000;
  const int screenHeight = 600;
  // window title "Communication Network"
  InitWindow(screenWidth, screenHeight, "Communication Network");
  SetTargetFPS(60);

  Vector2 positions[15];  // Max of 15 vertices
  calculateNodePositions(N, positions);

#ifdef MILESTONE3
  //  State variables for animation and button
  bool isPlaying = false;
  bool isFinished = false;
  int current_node_idx = 0;
  int current_jump = 0;
  float state_timer = 0.0f;
  int state = 1;  // 0 = Wait at node, 1 = Move on edge
  int current_edge_weight = 1;
  Vector2 buttonCenter = {920, 50};  // Fixed position to prevent cutoff
  float buttonRadius = 40.0f;        // Increased radius to fit text

  // Grab first edge weight for animation initialization
  if (path_len > 1) {
    Node* temp = graph[path[0]];
    while (temp) {
      if (temp->vertex == path[1]) {
        current_edge_weight = temp->weight;
        break;
      }
      temp = temp->next;
    }
  } else {
    isFinished = true;
  }
#endif

  // Main Screen Loop
  while (!WindowShouldClose()) {
#ifdef MILESTONE3
    // Logic to update animation and handle button clicks
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(mouse, buttonCenter, buttonRadius)) {
      if (!isFinished) isPlaying = !isPlaying;
    }

    if (isPlaying && !isFinished) {
      state_timer += GetFrameTime();
      if (state == 1) {  // Moving on edge
        if (state_timer >= 0.3f) {
          state_timer -= 0.3f;
          current_jump++;
          if (current_jump >= current_edge_weight) {
            current_node_idx++;
            current_jump = 0;
            if (current_node_idx >= path_len - 1) {
              isFinished = true;
              isPlaying = false;
            } else {
              state = 0;
              state_timer = 0.0f;  // Wait at node
              current_edge_weight = 1;
              Node* temp = graph[path[current_node_idx]];
              while (temp) {
                if (temp->vertex == path[current_node_idx + 1]) {
                  current_edge_weight = temp->weight;
                  break;
                }
                temp = temp->next;
              }
            }
          }
        }
      } else if (state == 0) {  // Waiting at node
        if (state_timer >= 1.0f) {
          state_timer -= 1.0f;
          state = 1;
        }
      }
    }
#endif

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw Edges and Weights
    // Edges first so they appear "under" the nodes
    for (int u = 0; u < N; u++) {
      Node* temp = graph[u];
      while (temp) {
        int v = temp->vertex;
        int weight = temp->weight;

        // Offset to the center of the rectangle (assuming rect is 40x40)
        Vector2 start = {positions[u].x + 20, positions[u].y + 20};
        Vector2 end = {positions[v].x + 20, positions[v].y + 20};

        // Calculate edge angle
        float angle = atan2(end.y - start.y, end.x - start.x);
        float nx = -sin(angle);
        float ny = cos(angle);
        float offset = 10.0f;  // Shift the line 10 pixels orthogonally so
                               // returning edges won't overlap

        // Apply the offset to both start and end points
        start.x += nx * offset;
        start.y += ny * offset;
        end.x += nx * offset;
        end.y += ny * offset;

        // Draw the connecting line using the shifted coordinates
        DrawLineEx(start, end, 2.0f, DARKGRAY);

        float arrow_length = 15.0f;

        // Move tip back slightly so it doesn't hide completely under the
        // destination rectangle
        Vector2 tip = {end.x - cos(angle) * 20, end.y - sin(angle) * 20};

        // Draw Arrowhead (Two lines forming a 'V')
        Vector2 p1 = {tip.x - arrow_length * cos(angle - PI / 6),
                      tip.y - arrow_length * sin(angle - PI / 6)};
        Vector2 p2 = {tip.x - arrow_length * cos(angle + PI / 6),
                      tip.y - arrow_length * sin(angle + PI / 6)};
        DrawLineEx(tip, p1, 2.0f, DARKGRAY);
        DrawLineEx(tip, p2, 2.0f, DARKGRAY);

        // Draw the weight text at the midpoint (small white rectangle with red
        // text)
        int midX = (start.x + end.x) / 2;
        int midY = (start.y + end.y) / 2;
        DrawRectangle(midX - 5, midY - 10, 20, 20,
                      RAYWHITE);  // Small background to make text readable
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
      DrawText(TextFormat("%d", i), positions[i].x + 12, positions[i].y + 10,
               20, BLACK);
    }

#ifdef MILESTONE3
    // Draw Play/Stop, Yellow Entity, and Text
    DrawPoly(buttonCenter, 6, buttonRadius, 0, RED);

    // Dynamically measure and center the text inside the button
    const char* btnText = isPlaying ? "STOP" : "PLAY";
    int textWidth = MeasureText(btnText, 20);
    DrawText(btnText, buttonCenter.x - (textWidth / 2), buttonCenter.y - 10, 20,
             WHITE);

    if (path_len > 0) {
      Vector2 entityPos;
      if (isFinished) {
        entityPos = positions[path[path_len - 1]];
      } else if (state == 0) {
        entityPos = positions[path[current_node_idx]];
      } else {
        Vector2 startPos = positions[path[current_node_idx]];
        Vector2 endPos = positions[path[current_node_idx + 1]];
        float angle = atan2(endPos.y - startPos.y, endPos.x - startPos.x);
        float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
        startPos.x += nx * offset;
        startPos.y += ny * offset;
        endPos.x += nx * offset;
        endPos.y += ny * offset;

        float fraction =
            (current_jump + (state_timer / 0.3f)) / (float)current_edge_weight;
        if (fraction > 1.0f) fraction = 1.0f;
        entityPos.x = startPos.x + (endPos.x - startPos.x) * fraction;
        entityPos.y = startPos.y + (endPos.y - startPos.y) * fraction;
      }
      DrawCircle(entityPos.x + 20, entityPos.y + 20, 15, YELLOW);
    }

    if (isFinished && path_len > 1) {
      DrawText("Reached Destination", screenWidth / 2 - 150, 20, 30, GREEN);
    } else if (path_len == 0) {
      DrawText("No Path Found", screenWidth / 2 - 100, 20, 30, RED);
    }
#endif

    EndDrawing();
  }

  CloseWindow();
}
