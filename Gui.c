#include <math.h>
#include <raylib.h>
#include <stdio.h>

#include "Dijkstra.h"

/** *-------1. Calculate graphic vertex layout:-------* */

void calculateNodePositions(Node** graph, int N, Vector2* positions) {
  (void)graph;
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
  calculateNodePositions(graph, N, positions);

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
// ==============================================================================
// ---> MILESTONE 4 ADDITION: Parallel Array Animation
// ==============================================================================
#ifdef MILESTONE4
#include <signal.h>
#include <sys/types.h>

// Struct to track the independent animation state of each traveler
typedef struct {
  int* path;
  int path_len;
  pid_t pid;
  bool isFinished;
  bool isKilled;  // True if the parent has sent the SIGTERM signal
  int current_node_idx;
  int current_jump;
  float state_timer;
  int state;  // 0 = Wait at node, 1 = Move on edge
  int current_edge_weight;
  Color color;
} Traveler;

void displayGraphGUI_M4(Node** graph, int N, int** paths, int* path_lens,
                        pid_t* pids, int num_travelers) {
  const int screenWidth = 1000;
  const int screenHeight = 600;
  InitWindow(screenWidth, screenHeight,
             "Communication Network - Multi-Process");
  SetTargetFPS(60);

  Vector2 positions[15];
  calculateNodePositions(graph, N, positions);

  bool isPlaying = false;
  Vector2 buttonCenter = {920, 50};
  float buttonRadius = 40.0f;

  // Array of distinct colors for the travelers
  Color palette[] = {YELLOW,  GREEN, MAGENTA, ORANGE, PURPLE,
                     SKYBLUE, PINK,  LIME,    GOLD,   VIOLET};

  Traveler* travelers = malloc(num_travelers * sizeof(Traveler));

  // Initialize all travelers
  for (int i = 0; i < num_travelers; i++) {
    travelers[i].path = paths[i];
    travelers[i].path_len = path_lens[i];
    travelers[i].pid = pids[i];
    travelers[i].isFinished = (path_lens[i] <= 1);
    travelers[i].isKilled = travelers[i].isFinished;
    travelers[i].current_node_idx = 0;
    travelers[i].current_jump = 0;
    travelers[i].state_timer = 0.0f;
    travelers[i].state = 1;
    travelers[i].color = palette[i % 10];  // Assign unique color

    if (travelers[i].path_len > 1) {
      Node* temp = graph[travelers[i].path[0]];
      travelers[i].current_edge_weight = 1;
      while (temp) {
        if (temp->vertex == travelers[i].path[1]) {
          travelers[i].current_edge_weight = temp->weight;
          break;
        }
        temp = temp->next;
      }
    } else if (!travelers[i].isKilled && travelers[i].pid > 0) {
      // Edge case: No path found, kill child immediately
      kill(travelers[i].pid, SIGTERM);
      travelers[i].isKilled = true;
    }
  }

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(mouse, buttonCenter, buttonRadius)) {
      isPlaying = !isPlaying;
    }

    bool allFinished = true;

    // Update loop for all travelers simultaneously
    if (isPlaying) {
      for (int i = 0; i < num_travelers; i++) {
        if (!travelers[i].isFinished) {
          allFinished = false;
          travelers[i].state_timer += GetFrameTime();

          if (travelers[i].state == 1) {  // Moving
            if (travelers[i].state_timer >= 0.3f) {
              travelers[i].state_timer -= 0.3f;
              travelers[i].current_jump++;

              if (travelers[i].current_jump >=
                  travelers[i].current_edge_weight) {
                travelers[i].current_node_idx++;
                travelers[i].current_jump = 0;

                if (travelers[i].current_node_idx >=
                    travelers[i].path_len - 1) {
                  travelers[i].isFinished = true;
                  // ---> PARENT SENDS SIGNAL TO TERMINATE CHILD
                  if (!travelers[i].isKilled) {
                    kill(travelers[i].pid, SIGTERM);
                    travelers[i].isKilled = true;
                  }
                } else {
                  travelers[i].state = 0;
                  travelers[i].state_timer = 0.0f;
                  travelers[i].current_edge_weight = 1;
                  Node* temp =
                      graph[travelers[i].path[travelers[i].current_node_idx]];
                  while (temp) {
                    if (temp->vertex ==
                        travelers[i].path[travelers[i].current_node_idx + 1]) {
                      travelers[i].current_edge_weight = temp->weight;
                      break;
                    }
                    temp = temp->next;
                  }
                }
              }
            }
          } else if (travelers[i].state == 0) {  // Waiting
            if (travelers[i].state_timer >= 1.0f) {
              travelers[i].state_timer -= 1.0f;
              travelers[i].state = 1;
            }
          }
        }
      }
    } else {
      // Just check if all are done to update the button text
      for (int i = 0; i < num_travelers; i++) {
        if (!travelers[i].isFinished) allFinished = false;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw Edges and Weights
    for (int u = 0; u < N; u++) {
      Node* temp = graph[u];
      while (temp) {
        int v = temp->vertex;
        int weight = temp->weight;
        Vector2 start = {positions[u].x + 20, positions[u].y + 20};
        Vector2 end = {positions[v].x + 20, positions[v].y + 20};

        float angle = atan2(end.y - start.y, end.x - start.x);
        float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
        start.x += nx * offset;
        start.y += ny * offset;
        end.x += nx * offset;
        end.y += ny * offset;

        DrawLineEx(start, end, 2.0f, DARKGRAY);

        float arrow_length = 15.0f;
        Vector2 tip = {end.x - cos(angle) * 20, end.y - sin(angle) * 20};
        Vector2 p1 = {tip.x - arrow_length * cos(angle - PI / 6),
                      tip.y - arrow_length * sin(angle - PI / 6)};
        Vector2 p2 = {tip.x - arrow_length * cos(angle + PI / 6),
                      tip.y - arrow_length * sin(angle + PI / 6)};
        DrawLineEx(tip, p1, 2.0f, DARKGRAY);
        DrawLineEx(tip, p2, 2.0f, DARKGRAY);

        int midX = (start.x + end.x) / 2;
        int midY = (start.y + end.y) / 2;
        DrawRectangle(midX - 5, midY - 10, 20, 20, RAYWHITE);
        DrawText(TextFormat("%d", weight), midX, midY - 10, 20, RED);

        temp = temp->next;
      }
    }

    // Draw Nodes
    for (int i = 0; i < N; i++) {
      DrawRectangle(positions[i].x, positions[i].y, 40, 40, SKYBLUE);
      DrawRectangleLines(positions[i].x, positions[i].y, 40, 40, BLUE);
      DrawText(TextFormat("%d", i), positions[i].x + 12, positions[i].y + 10,
               20, BLACK);
    }

    // Draw Travelers
    for (int i = 0; i < num_travelers; i++) {
      if (travelers[i].path_len > 0) {
        Vector2 entityPos;
        if (travelers[i].isFinished) {
          entityPos = positions[travelers[i].path[travelers[i].path_len - 1]];
        } else if (travelers[i].state == 0) {
          entityPos =
              positions[travelers[i].path[travelers[i].current_node_idx]];
        } else {
          Vector2 startPos =
              positions[travelers[i].path[travelers[i].current_node_idx]];
          Vector2 endPos =
              positions[travelers[i].path[travelers[i].current_node_idx + 1]];
          float angle = atan2(endPos.y - startPos.y, endPos.x - startPos.x);
          float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
          startPos.x += nx * offset;
          startPos.y += ny * offset;
          endPos.x += nx * offset;
          endPos.y += ny * offset;

          float fraction =
              (travelers[i].current_jump + (travelers[i].state_timer / 0.3f)) /
              (float)travelers[i].current_edge_weight;
          if (fraction > 1.0f) fraction = 1.0f;
          entityPos.x = startPos.x + (endPos.x - startPos.x) * fraction;
          entityPos.y = startPos.y + (endPos.y - startPos.y) * fraction;
        }

        // Add a slight visual offset so concurrent dots don't perfectly hide
        // each other on the same node
        float dotOffset_x = (i % 2 == 0) ? (i * 3.0f) : -(i * 3.0f);
        float dotOffset_y = (i % 3 == 0) ? (i * 3.0f) : -(i * 3.0f);

        // Draw smaller, multi-colored circles with borders
        DrawCircle(entityPos.x + 20 + dotOffset_x,
                   entityPos.y + 20 + dotOffset_y, 10, travelers[i].color);
        DrawCircleLines(entityPos.x + 20 + dotOffset_x,
                        entityPos.y + 20 + dotOffset_y, 10, BLACK);
      }
    }

    // Draw Button and Status
    DrawPoly(buttonCenter, 6, buttonRadius, 0, RED);
    if (allFinished) {
      isPlaying = false;  // Force stop when all are done
      DrawText("DONE", buttonCenter.x - MeasureText("DONE", 20) / 2,
               buttonCenter.y - 10, 20, WHITE);
      DrawText("All Travelers Finished", screenWidth / 2 - 160, 20, 30, GREEN);
    } else {
      const char* btnText = isPlaying ? "STOP" : "PLAY";
      DrawText(btnText, buttonCenter.x - (MeasureText(btnText, 20) / 2),
               buttonCenter.y - 10, 20, WHITE);
    }

    EndDrawing();
  }

  free(travelers);
  CloseWindow();
}
#endif

// ==============================================================================
// ---> MILESTONE 5 ADDITION: IPC Parent Listener & Buffer GUI
// ==============================================================================
#ifdef MILESTONE5
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
  pid_t pid;
  int current_node;
  int next_node;
  bool is_destination;
} TravelerMsg;

// Upgraded GUI struct: Caches the path and tracks what has been printed
typedef struct {
  pid_t pid;
  int path[30];
  int path_len;
  bool isIPCFinished;

  // M4 Animation Variables
  int current_node_idx;
  int current_jump;
  float state_timer;
  int state;
  int current_edge_weight;
  Color color;

  // Tracks terminal printing sync
  int printed_node_idx;
} TravelerGUI;

void displayGraphGUI_M5(Node** graph, int N, int read_fd, int num_travelers) {
  const int screenWidth = 1000;
  const int screenHeight = 600;
  InitWindow(screenWidth, screenHeight,
             "Communication Network - IPC Buffering (M5)");
  SetTargetFPS(60);

  // Set pipe to non-blocking so read() doesn't freeze the Raylib GUI while
  // waiting for children
  int flags = fcntl(read_fd, F_GETFL, 0);
  fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

  Vector2 positions[15];
  calculateNodePositions(graph, N, positions);

  Color palette[] = {YELLOW,  GREEN, MAGENTA, ORANGE, PURPLE,
                     SKYBLUE, PINK,  LIME,    GOLD,   VIOLET};

  TravelerGUI* travelers = calloc(num_travelers, sizeof(TravelerGUI));

  int node_owner[15];

  for (int i = 0; i < 15; i++) {
    node_owner[i] = -1;
  }

  for (int i = 0; i < num_travelers; i++) {
    travelers[i].color = palette[i % 10];
    travelers[i].state = 0;
    travelers[i].current_edge_weight = 1;
    // calloc already sets printed_node_idx to 0, which is perfect
  }

  bool isPlaying = false;
  Vector2 buttonCenter = {920, 50};
  float buttonRadius = 40.0f;

  while (!WindowShouldClose()) {
    // --- 1. UI Interactions ---
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointCircle(mouse, buttonCenter, buttonRadius)) {
      isPlaying = !isPlaying;
    }

    // --- 2. Read IPC Messages (SILENTLY Buffer the Path) ---
    TravelerMsg msg;
    ssize_t bytes_read;
    while ((bytes_read = read(read_fd, &msg, sizeof(TravelerMsg))) > 0) {
      if (bytes_read != sizeof(TravelerMsg)) continue;

      int idx = -1;
      for (int i = 0; i < num_travelers; i++) {
        if (travelers[i].pid == msg.pid || travelers[i].pid == 0) {
          idx = i;
          travelers[i].pid = msg.pid;
          break;
        }
      }

      if (idx != -1) {
        if (travelers[idx].path_len == 0) {
          travelers[idx].path[0] = msg.current_node;
          travelers[idx].path_len = 1;

          if (node_owner[msg.current_node] == -1) {
            node_owner[msg.current_node] = idx;
          }
        }

        if (!msg.is_destination) {
          travelers[idx].path[travelers[idx].path_len] = msg.next_node;
          travelers[idx].path_len++;
          // Removed the printf from here!
        } else {
          travelers[idx].isIPCFinished = true;
          // Removed the printf from here!
        }
      }
    }

    // --- 3. Run M4 Animation Logic (Only if Playing) ---
    bool all_gui_finished = true;

    for (int i = 0; i < num_travelers; i++) {
      if (travelers[i].pid == 0) {
        all_gui_finished = false;
        continue;
      }

      bool gui_reached_end =
          (travelers[i].current_node_idx >= travelers[i].path_len - 1);
      if (!gui_reached_end || !travelers[i].isIPCFinished) {
        all_gui_finished = false;
      }

      if (isPlaying) {
        // ---> NEW LOGIC: Print synced with GUI arrival <---
        // If the traveler is resting at a node (state == 0) and hasn't printed
        // yet:
        if (travelers[i].state == 0 &&
            travelers[i].printed_node_idx == travelers[i].current_node_idx) {
          if (travelers[i].current_node_idx < travelers[i].path_len - 1) {
            printf("[%d] arrived at node %d | next node: %d\n",
                   travelers[i].pid,
                   travelers[i].path[travelers[i].current_node_idx],
                   travelers[i].path[travelers[i].current_node_idx + 1]);
            travelers[i].printed_node_idx++;
          } else if (travelers[i].isIPCFinished) {
            printf("[%d] arrived at node %d | DESTINATION\n", travelers[i].pid,
                   travelers[i].path[travelers[i].current_node_idx]);
            printf("[%d] finished\n", travelers[i].pid);

            node_owner[travelers[i].path[travelers[i].current_node_idx]] = -1;

            travelers[i].printed_node_idx++;
          }
        }

        // ---> ANIMATION UPDATES <---
        if (!gui_reached_end) {
          travelers[i].state_timer += GetFrameTime();

          if (travelers[i].state == 1) {  // Moving
            if (travelers[i].state_timer >= 0.3f) {
              travelers[i].state_timer -= 0.3f;
              travelers[i].current_jump++;

              if (travelers[i].current_jump >=
                  travelers[i].current_edge_weight) {
                travelers[i].current_node_idx++;
                travelers[i].current_jump = 0;
                travelers[i].state =
                    0;  // Set to resting, which triggers the next log!
                travelers[i].state_timer = 0.0f;
              }
            }
          } else if (travelers[i].state == 0) {  // Waiting
            if (travelers[i].state_timer >= 1.0f) {

              int currentNode = travelers[i].path[travelers[i].current_node_idx];
              int nextNode = travelers[i].path[travelers[i].current_node_idx + 1];
              if (node_owner[nextNode] != -1 && node_owner[nextNode] != i) {
                travelers[i].state_timer = 0.0f;
                continue;
              }
              node_owner[nextNode] = i;
              node_owner[currentNode] = -1;

              travelers[i].state_timer -= 1.0f;
              travelers[i].state = 1;

              travelers[i].current_edge_weight = 1;
              Node* temp =
                  graph[travelers[i].path[travelers[i].current_node_idx]];
              while (temp) {
                if (temp->vertex ==
                    travelers[i].path[travelers[i].current_node_idx + 1]) {
                  travelers[i].current_edge_weight = temp->weight;
                  break;
                }
                temp = temp->next;
              }
            }
          }
        }
      }
    }

    if (all_gui_finished) isPlaying = false;

    // --- 4. Draw Frame ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw Edges
    for (int u = 0; u < N; u++) {
      Node* temp = graph[u];
      while (temp) {
        int v = temp->vertex;
        Vector2 start = {positions[u].x + 20, positions[u].y + 20};
        Vector2 end = {positions[v].x + 20, positions[v].y + 20};

        float angle = atan2(end.y - start.y, end.x - start.x);
        float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
        start.x += nx * offset;
        start.y += ny * offset;
        end.x += nx * offset;
        end.y += ny * offset;

        DrawLineEx(start, end, 2.0f, DARKGRAY);

        float arrow_length = 15.0f;
        Vector2 tip = {end.x - cos(angle) * 20, end.y - sin(angle) * 20};
        Vector2 p1 = {tip.x - arrow_length * cos(angle - PI / 6),
                      tip.y - arrow_length * sin(angle - PI / 6)};
        Vector2 p2 = {tip.x - arrow_length * cos(angle + PI / 6),
                      tip.y - arrow_length * sin(angle + PI / 6)};
        DrawLineEx(tip, p1, 2.0f, DARKGRAY);
        DrawLineEx(tip, p2, 2.0f, DARKGRAY);

        int midX = (start.x + end.x) / 2;
        int midY = (start.y + end.y) / 2;
        DrawRectangle(midX - 5, midY - 10, 20, 20, RAYWHITE);
        DrawText(TextFormat("%d", temp->weight), midX, midY - 10, 20, RED);

        temp = temp->next;
      }
    }

    // Draw Nodes
    for (int i = 0; i < N; i++) {
      DrawRectangle(positions[i].x, positions[i].y, 40, 40, SKYBLUE);
      DrawRectangleLines(positions[i].x, positions[i].y, 40, 40, BLUE);
      DrawText(TextFormat("%d", i), positions[i].x + 12, positions[i].y + 10,
               20, BLACK);
    }

    // Draw Travelers
    for (int i = 0; i < num_travelers; i++) {
      if (travelers[i].pid != 0 && travelers[i].path_len > 0) {
        Vector2 entityPos;

        if (travelers[i].current_node_idx >= travelers[i].path_len - 1) {
          entityPos = positions[travelers[i].path[travelers[i].path_len - 1]];
        } else if (travelers[i].state == 0) {
          entityPos =
              positions[travelers[i].path[travelers[i].current_node_idx]];
        } else {
          Vector2 startPos =
              positions[travelers[i].path[travelers[i].current_node_idx]];
          Vector2 endPos =
              positions[travelers[i].path[travelers[i].current_node_idx + 1]];

          float angle = atan2(endPos.y - startPos.y, endPos.x - startPos.x);
          float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
          startPos.x += nx * offset;
          startPos.y += ny * offset;
          endPos.x += nx * offset;
          endPos.y += ny * offset;

          float fraction =
              (travelers[i].current_jump + (travelers[i].state_timer / 0.3f)) /
              (float)travelers[i].current_edge_weight;
          if (fraction > 1.0f) fraction = 1.0f;

          entityPos.x = startPos.x + (endPos.x - startPos.x) * fraction;
          entityPos.y = startPos.y + (endPos.y - startPos.y) * fraction;
        }

        float dotOffset_x = (i % 2 == 0) ? (i * 3.0f) : -(i * 3.0f);
        float dotOffset_y = (i % 3 == 0) ? (i * 3.0f) : -(i * 3.0f);

        DrawCircle(entityPos.x + 20 + dotOffset_x,
                   entityPos.y + 20 + dotOffset_y, 10, travelers[i].color);
        DrawCircleLines(entityPos.x + 20 + dotOffset_x,
                        entityPos.y + 20 + dotOffset_y, 10, BLACK);
      }
    }

    // Draw Play/Stop Button
    DrawPoly(buttonCenter, 6, buttonRadius, 0, RED);
    if (all_gui_finished) {
      DrawText("DONE", buttonCenter.x - MeasureText("DONE", 20) / 2,
               buttonCenter.y - 10, 20, WHITE);
      DrawText("All Travelers Finished", screenWidth / 2 - 160, 20, 30, GREEN);
    } else {
      const char* btnText = isPlaying ? "STOP" : "PLAY";
      DrawText(btnText, buttonCenter.x - (MeasureText(btnText, 20) / 2),
               buttonCenter.y - 10, 20, WHITE);
    }

    EndDrawing();
  }

  free(travelers);
  CloseWindow();
}
#endif