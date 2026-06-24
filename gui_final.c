#include "gui_final.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Dijkstra.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct {
    int traveler_idx;
    int job_len;
} WaitRequest;

typedef struct {
  pid_t pid;
  int path[30];
  int intended_next[30];
  int path_len;
  bool isIPCFinished;

  int current_node_idx;
  int current_jump;
  float state_timer;
  int state;
  int current_edge_weight;
  Color color;

  int printed_node_idx;
} TravelerGUI;

void calculateNodePositions(Node** graph, int N, Vector2* positions) {
  (void)graph;
  float screen_w = 1000.0f;
  float screen_h = 600.0f;
  float center_x = screen_w / 2.0f;
  float center_y = screen_h / 2.0f;
  float radius = 220.0f;

  for (int i = 0; i < N; i++) {
    float angle = (2.0f * PI * i) / N;
    positions[i].x = center_x + radius * cos(angle) - 20;
    positions[i].y = center_y + radius * sin(angle) - 20;
  }
}

void displayGraphGUI_M6(Node** graph, int N, int read_fd, int num_travelers, const char* algo, sem_t** go_sems) {
  const int screenWidth = 1000;
  const int screenHeight = 600;
  InitWindow(screenWidth, screenHeight, "Communication Network - IPC Scheduler (M7)");
  SetTargetFPS(60);

  int flags = fcntl(read_fd, F_GETFL, 0);
  fcntl(read_fd, F_SETFL, flags | O_NONBLOCK);

  Vector2 positions[30];
  calculateNodePositions(graph, N, positions);

  Color palette[] = {YELLOW, GREEN, MAGENTA, ORANGE, PURPLE, SKYBLUE, PINK, LIME, GOLD, VIOLET};

  TravelerGUI* travelers = calloc(num_travelers, sizeof(TravelerGUI));

  int* os_node_owner = malloc(N * sizeof(int));
  int* visual_node_owner = malloc(N * sizeof(int));

  for (int i = 0; i < N; i++) {
    os_node_owner[i] = -1;
    visual_node_owner[i] = -1;
  }

  WaitRequest** wait_queues = malloc(N * sizeof(WaitRequest*));
  int* wait_counts = calloc(N, sizeof(int));
  for (int i = 0; i < N; i++) {
      wait_queues[i] = malloc(num_travelers * sizeof(WaitRequest));
  }

  for (int i = 0; i < num_travelers; i++) {
    travelers[i].color = palette[i % 10];
    travelers[i].state = 0;
    travelers[i].current_edge_weight = 1;
    travelers[i].state_timer = 0.0f;
  }

  // חזרנו למצב המקורי: המשחק מתחיל בסטופ וממתין ללחיצה על כפתור ה-Start
  bool isPlaying = false;
  Vector2 buttonCenter = {920, 50};
  float buttonRadius = 40.0f;

  while (!WindowShouldClose()) {
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointCircle(mouse, buttonCenter, buttonRadius)) {
      isPlaying = !isPlaying;
    }

    TravelerMsg msg;
    ssize_t bytes_read;

    // --- PHASE 1: DRAIN THE PIPE ---
    // קריאת ה-Pipe רצה תמיד ברקע כדי למנוע הצטברות הודעות ואיבוד סנכרון בזמן שהמשחק ב-Stop
    while ((bytes_read = read(read_fd, &msg, sizeof(TravelerMsg))) > 0) {
      if (bytes_read != sizeof(TravelerMsg)) continue;
      if (msg.action == ACTION_REQ_NODE) {
          int node = msg.node_id;
          int t_idx = msg.traveler_idx;
          printf("[REQUEST] traveler=%d node=%d job_len=%d\n", t_idx, node, msg.job_len);

          wait_queues[node][wait_counts[node]].traveler_idx = t_idx;
          wait_queues[node][wait_counts[node]].job_len = msg.job_len;
          wait_counts[node]++;
      }
      else if (msg.action == ACTION_RELEASE_NODE) {
          int node = msg.node_id;
          os_node_owner[node] = -1;
      }
      else if (msg.action == ACTION_UPDATE_GUI) {
        int idx = msg.traveler_idx;
        travelers[idx].pid = msg.pid;

        if (travelers[idx].path_len == 0) {
          visual_node_owner[msg.node_id] = idx;
        }
        travelers[idx].path[travelers[idx].path_len] = msg.node_id;
        travelers[idx].intended_next[travelers[idx].path_len] = msg.intended_next_node;
        travelers[idx].path_len++;
        if (msg.is_destination) travelers[idx].isIPCFinished = true;
      }
    }

    // --- PHASE 2: SCHEDULER ALLOCATION ---
    // גם הקצאות המתזמן רצות תמיד ברקע כדי שהילדים ב-Backend לא יתקעו בזמן שהמסך בסטופ
    for (int node = 0; node < N; node++) {
        if (os_node_owner[node] == -1 && wait_counts[node] > 0) {
            int selected_idx = 0;

            if (strcmp(algo, "sjf") == 0) {
                if (wait_counts[node] > 1) {
                    printf("\n--- SJF DECISION FOR NODE %d ---\n", node);
                    for (int k = 0; k < wait_counts[node]; k++) {
                        printf(" Waiting: Traveler=%d, Job_Len=%d\n", wait_queues[node][k].traveler_idx, wait_queues[node][k].job_len);
                    }
                }

                int min_job = wait_queues[node][0].job_len;
                for (int k = 1; k < wait_counts[node]; k++) {
                    if (wait_queues[node][k].job_len < min_job) {
                        min_job = wait_queues[node][k].job_len;
                        selected_idx = k;
                    }
                }

                if (wait_counts[node] > 1) {
                    printf(" -> [SJF PICKED] Traveler=%d with Job_Len=%d\n------------------------------\n",
                           wait_queues[node][selected_idx].traveler_idx, wait_queues[node][selected_idx].job_len);
                }
            }

            int next_traveler = wait_queues[node][selected_idx].traveler_idx;

            for (int k = selected_idx; k < wait_counts[node] - 1; k++) {
                wait_queues[node][k] = wait_queues[node][k + 1];
            }
            wait_counts[node]--;

            os_node_owner[node] = next_traveler;
            sem_post(go_sems[next_traveler]);
        }
    }

    bool all_gui_finished = true;

    for (int i = 0; i < num_travelers; i++) {
      if (travelers[i].pid == 0) {
        all_gui_finished = false;
        continue;
      }

      bool gui_reached_end = (travelers[i].current_node_idx >= travelers[i].path_len - 1);
      if (!gui_reached_end || !travelers[i].isIPCFinished) {
        all_gui_finished = false;
      }

      // האנימציה והשהיית הזמן מקודמות אך ורק אם לחצנו על PLAY
      if (isPlaying) {
        // הדפסת הלוגים מסונכרנת עם הגעת העיגול לקודקוד
        if (travelers[i].state == 0 && travelers[i].printed_node_idx == travelers[i].current_node_idx) {
            int p_idx = travelers[i].current_node_idx;
            if (!travelers[i].isIPCFinished || p_idx < travelers[i].path_len - 1) {
                printf("[%d] arrived at node %d | next node: %d\n",
                       travelers[i].pid, travelers[i].path[p_idx], travelers[i].intended_next[p_idx]);
                travelers[i].printed_node_idx++;
            }
          else if (travelers[i].isIPCFinished) {
              printf("[%d] arrived at node %d | DESTINATION\n", travelers[i].pid, travelers[i].path[p_idx]);
              printf("[%d] finished\n", travelers[i].pid);
              visual_node_owner[travelers[i].path[p_idx]] = -1;
              travelers[i].printed_node_idx++;
            }
        }

        if (!gui_reached_end) {
          travelers[i].state_timer += GetFrameTime();

          if (travelers[i].state == 1) {
            if (travelers[i].state_timer >= 0.3f) {
              travelers[i].state_timer -= 0.3f;
              travelers[i].current_jump++;

              if (travelers[i].current_jump >= travelers[i].current_edge_weight) {
                travelers[i].current_node_idx++;
                travelers[i].current_jump = 0;
                travelers[i].state = 0;
                travelers[i].state_timer = 0.0f; // איפוס הטיימר לטובת שהיית ה-1 שניה בצומת
              }
            }
          } else if (travelers[i].state == 0) {
            // 1. הגבלת הטיימר לעצירה מוחלטת ב-1.0 שניות כדי למנוע צבירת זמן עודף
            if (travelers[i].state_timer > 1.0f) {
              travelers[i].state_timer = 1.0f;
            }

            // 2. בדיקה: האם עברה שנייה שלמה וגם ה-Backend שלח אישור (הצומת הבא מעודכן)?
            if (travelers[i].state_timer >= 1.0f && travelers[i].current_node_idx + 1 < travelers[i].path_len) {
              int currentNode = travelers[i].path[travelers[i].current_node_idx];
              int nextNode = travelers[i].path[travelers[i].current_node_idx + 1];

              // בדיקת נעילה ויזואלית
              if (visual_node_owner[nextNode] != -1 && visual_node_owner[nextNode] != i) {
                continue;
              }

              visual_node_owner[nextNode] = i;
              visual_node_owner[currentNode] = -1;

              travelers[i].current_edge_weight = 1;
              Node* temp = graph[travelers[i].path[travelers[i].current_node_idx]];
              while (temp) {
                if (temp->vertex == travelers[i].path[travelers[i].current_node_idx + 1]) {
                  travelers[i].current_edge_weight = temp->weight;
                  break;
                }
                temp = temp->next;
              }

              // 3. איפוס מוחלט ל-0 כדי שהאנימציה של הנסיעה ב-state 1 תתחיל בדיוק בזמן ובצורה חלקה
              travelers[i].state_timer = 0.0f;
              travelers[i].state = 1;
            }
          }
        }
      }
    }

    if (all_gui_finished) isPlaying = false;

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
        Vector2 p1 = {tip.x - arrow_length * cos(angle - PI / 6), tip.y - arrow_length * sin(angle - PI / 6)};
        Vector2 p2 = {tip.x - arrow_length * cos(angle + PI / 6), tip.y - arrow_length * sin(angle + PI / 6)};
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
      DrawText(TextFormat("%d", i), positions[i].x + 12, positions[i].y + 10, 20, BLACK);
    }

    // Draw Travelers
    for (int i = 0; i < num_travelers; i++) {
      if (travelers[i].pid != 0 && travelers[i].path_len > 0) {
        Vector2 entityPos;

        if (travelers[i].current_node_idx >= travelers[i].path_len - 1) {
          entityPos = positions[travelers[i].path[travelers[i].path_len - 1]];
        } else if (travelers[i].state == 0) {
          entityPos = positions[travelers[i].path[travelers[i].current_node_idx]];
        } else {
          Vector2 startPos = positions[travelers[i].path[travelers[i].current_node_idx]];
          Vector2 endPos = positions[travelers[i].path[travelers[i].current_node_idx + 1]];

          float angle = atan2(endPos.y - startPos.y, endPos.x - startPos.x);
          float nx = -sin(angle), ny = cos(angle), offset = 10.0f;
          startPos.x += nx * offset;
          startPos.y += ny * offset;
          endPos.x += nx * offset;
          endPos.y += ny * offset;

          float fraction = (travelers[i].current_jump + (travelers[i].state_timer / 0.3f)) / (float)travelers[i].current_edge_weight;
          if (fraction > 1.0f) fraction = 1.0f;

          entityPos.x = startPos.x + (endPos.x - startPos.x) * fraction;
          entityPos.y = startPos.y + (endPos.y - startPos.y) * fraction;
        }

        float dotOffset_x = (i % 2 == 0) ? (i * 3.0f) : -(i * 3.0f);
        float dotOffset_y = (i % 3 == 0) ? (i * 3.0f) : -(i * 3.0f);

        DrawCircle(entityPos.x + 20 + dotOffset_x, entityPos.y + 20 + dotOffset_y, 10, travelers[i].color);
        DrawCircleLines(entityPos.x + 20 + dotOffset_x, entityPos.y + 20 + dotOffset_y, 10, BLACK);
      }
    }

    // Draw UI
    DrawPoly(buttonCenter, 6, buttonRadius, 0, RED);
    DrawText(TextFormat("Algo: %s", algo), 20, 20, 20, DARKBLUE);
    if (all_gui_finished) {
      DrawText("DONE", buttonCenter.x - MeasureText("DONE", 20) / 2, buttonCenter.y - 10, 20, WHITE);
      DrawText("All Travelers Finished", screenWidth / 2 - 160, 20, 30, GREEN);
    } else {
      const char* btnText = isPlaying ? "STOP" : "PLAY";
      DrawText(btnText, buttonCenter.x - (MeasureText(btnText, 20) / 2), buttonCenter.y - 10, 20, WHITE);
    }

    EndDrawing();
  }

  // Cleanup
  free(os_node_owner);
  free(visual_node_owner);
  for (int i = 0; i < N; i++) free(wait_queues[i]);
  free(wait_queues);
  free(wait_counts);
  free(travelers);
  CloseWindow();
}