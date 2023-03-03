CFLAGS = -Wall -Wextra -Werror -O3
CC = gcc
SRC = src

.PHONY: clean debug lint play release

sysprak-client: sysprak-client.o performConnection.o config.o sharedMemory.o prologPhase.o gamePhase.o network.o thinker.o king_logic.o man_logic.o thinker_common.o
	$(CC) $(CFLAGS) -o $@ $^

sysprak-client.o: $(SRC)/sysprak-client.c $(SRC)/performConnection.h
	$(CC) $(CFLAGS) -c $(SRC)/sysprak-client.c

performConnection.o: $(SRC)/performConnection.c $(SRC)/performConnection.h
	$(CC) $(CFLAGS) -c $(SRC)/performConnection.c

prologPhase.o: $(SRC)/prologPhase.c $(SRC)/prologPhase.h
	$(CC) $(CFLAGS) -c $(SRC)/prologPhase.c

gamePhase.o: $(SRC)/gamePhase.c $(SRC)/gamePhase.h
	$(CC) $(CFLAGS) -c $(SRC)/gamePhase.c

thinker.o: $(SRC)/thinker.c $(SRC)/thinker.h
	$(CC) $(CFLAGS) -c $(SRC)/thinker.c

king_logic.o: $(SRC)/king_logic.c $(SRC)/king_logic.h
	$(CC) $(CFLAGS) -c $(SRC)/king_logic.c

man_logic.o: $(SRC)/man_logic.c $(SRC)/man_logic.h
	$(CC) $(CFLAGS) -c $(SRC)/man_logic.c

# LEAF DEPENDENCIES

sharedMemory.o: $(SRC)/sharedMemory.c $(SRC)/sharedMemory.h
	$(CC) $(CFLAGS) -c $(SRC)/sharedMemory.c

config.o: $(SRC)/config.c $(SRC)/config.h
	$(CC) $(CFLAGS) -c $(SRC)/config.c

network.o: $(SRC)/network.c $(SRC)/network.h
	$(CC) $(CFLAGS) -c $(SRC)/network.c

thinker_common.o: $(SRC)/thinker_common.c $(SRC)/thinker_common.h
	$(CC) $(CFLAGS) -c $(SRC)/thinker_common.c

# PHONY TARGETS

debug: CFLAGS += -DDEBUG -g
debug: clean sysprak-client
	mv sysprak-client sysprak-client-debug

play: sysprak-client
	./sysprak-client

clean:
	rm -f *.o sysprak-client sysprak-client-debug abgabe.zip

release: clean
	zip -r abgabe.zip . -x @.zip-excludes

lint:
	cd src && cpplint --recursive --quiet \
	  --filter='-legal/copyright,-readability/casting,-build/header_guard,-build/include' .
