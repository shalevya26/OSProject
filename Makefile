CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99
GUI_LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

.PHONY: all clean milestone1 milestone2 milestone3 milestone4

all: milestone1 milestone2 milestone3 milestone4

milestone1:
	$(CC) $(CFLAGS) -o dijkstra main.c Dijkstra.c

milestone2:
	$(CC) $(CFLAGS) -DENABLE_GUI -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)

milestone3:
	$(CC) $(CFLAGS) -DENABLE_GUI -DMILESTONE3 -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)

milestone4:
	$(CC) $(CFLAGS) -DENABLE_GUI -DMILESTONE4 -o sim main.c Dijkstra.c Gui.c $(GUI_LDFLAGS)

clean:
	rm -f dijkstra sim
