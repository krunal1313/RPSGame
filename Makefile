SOURCE=main.cpp player.cpp server.cpp
MYPROGRAM=RPSGame

MYLIBRARIES=-lredox_static -lev -lhiredis -pthread
CC=g++ -std=c++11

#------------------------------------------------------------------------------



all: $(MYPROGRAM)



$(MYPROGRAM): $(SOURCE)

	$(CC) $(MYINCLUDES) $(SOURCE) -o $(MYPROGRAM) $(MYLIBRARIES)

clean:

	rm -f $(MYPROGRAM)


