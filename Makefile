CC = gcc

CFLAGS = -Wall -Wextra -std=gnu99
GUI_LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all clean milestone1 milestone2 milestone3 milestone4

all: milestone1 milestone2 milestone3 milestone4

CFLAGS = -Wall -Wextra -std=c99
GUI_LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all clean milestone1 milestone2 milestone3

all: milestone1 milestone2 milestone3


milestone1:
	$(CC) $(CFLAGS) -o dijkstra main.c Dijkstra.c

milestone2:
	$(CC) $(CFLAGS) -DENABLE_GUI -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)


milestone3:
	$(CC) $(CFLAGS) -DENABLE_GUI -DMILESTONE3 -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)

milestone4:
	$(CC) $(CFLAGS) -DENABLE_GUI -DMILESTONE4 -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)


# ---> MILESTONE 3 FIX: Added -DMILESTONE3 flag so the C code knows which version to build
milestone3:
	$(CC) $(CFLAGS) -DENABLE_GUI -DMILESTONE3 -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)


clean:
	rm -f dijkstra sim
