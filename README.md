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
