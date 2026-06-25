* Milestone 1 commands:

-compilation: make milestone1

-running: ./dijkstra <file_name>

-Description: The core Dijkastra algorithem. finds the shortest path between
2 vertexes on a wheighted, directed graph. given by a formated .txt file.

* Milestones 2 commands:

-compilation: make milestone2

-running: ./sim <file_name>

-Description: Static display of the given graph using raylib GUI. Our graph
resembles a communication network, displayed in a circular layout of devices (vertexes)

* Milestone 3 commands:

-compilation : make milestone3

-running : ./sim <file_name>

-Description: Simulates movement on the given graph, from 2 vertexes found via Dijkstra,
a yellow entity moves on the shortest path between them. as the edges it passes weight more,its
movement slows accordingly.

* Milestone 4 commands:

-compilation : make milestone4

-running : ./sim <file_name>

-Description: Simulates movement of multiple travelers, using fork() , we create son processes. the main process handles
their dijkstra-algorithmic and GUI display math calculations.

* Milestone 5 commands:

-compilation : make milestone5

-running : ./sim <file_name>

-Description: Simulates autonomous movement of multiple travelers. Son processes calculate their own paths and report to
the parent via IPC pipes. Pipes were chosen over shared memory because they provide built-in synchronization without
semaphores and perfectly fit our one-way (child-to-parent) data flow.

* Milestone 6 commands:

-compilation : make milestone6

-running : ./sim <file_name>

-Description: Adds synchronization between multiple travelers. Each vertex can contain only one traveler at a time.
If a traveler wants to enter a vertex that is already occupied, it waits until the vertex becomes free.
This prevents two travelers from being displayed on the same vertex at the same time.
The synchronization is handled using semaphores, while the GUI clearly shows the waiting behavior.

* Milestone 7 commands:

-compilation : make milestone7

-running : ./sim -schd fcfs <file_name>  OR  ./sim -schd sjf <file_name>

-Description: Replaces arbitrary vertex entry with a centralized Parent Scheduler using FCFS or SJF algorithms.
Processes request access through IPC pipes and block on personalized wake-up semaphores until the parent grants entrance based on the chosen scheduling policy.
We chose FCFS and SJF to provide a classic architectural comparison between a completely chronological baseline and a shortest-remaining-path optimization.

Team Contributions:

* Milestone 1: Developed collaboratively by Rotem and Shalev.
* Milestones 2–4: Developed by Itamar.
* Milestone 5: Developed by Shalev.
* Milestone 6: Developed by Rotem.
* Milestone 7: Developed collaboratively by Rotem, Shalev and Itamar.
* -push test-