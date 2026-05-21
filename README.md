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

  -running : ./sim

  -Description: Simulates movement on the given graph, from 2 vertexes found via Dijkstra,
   a yellow entity moves on the shortest path between them. as the edges it pasess wheight more,its
   movement slows accordintly.

   *Milestone 4 commands:
   
   -compilation: make milestone4
   
   -running: ./sim <file_name>

   Description: Adds multiple travalers to the simulation, each travaler is a separete prosses and each prosses
   is a son handeled by one mutual father prosses, the father does the dijkstra and display calaculations of each
   one of them and displays them running simultaneously.
