COMPILE = $(CC) $(CFLAGS) -Wall -c
LINK = $(CC) -Wall

EXECUTABLES = test test_lockfree_fifo

test_OBJECTS = test.o sound_api.o sine_generator.o sawtooth_generator.o lohi_generator.o print_time.o lockfree_fifo.o

test_lockfree_fifo_OBJECTS = test_lockfree_fifo.o

test_LIBS =
sound_api_LIBS = -lasound -lpthread
sine_generator_LIBS = -lm
sawtooth_generator_LIBS =
lohi_generator_LIBS = -lm
print_time_LIBS = -lrt
test_lockfree_fifo_LIBS = -lpthread

all: test
tests: test_lockfree_fifo

test: $(test_OBJECTS)
	$(LINK) $(test_LIBS) $(sound_api_LIBS) $(sine_generator_LIBS) $(sawtooth_generator_LIBS) $(lohi_generator_LIBS) $(print_time_LIBS) -o $@ $^

test_lockfree_fifo: lockfree_fifo.o test_lockfree_fifo.o
	$(LINK) $(lockfree_fifo_LIBS) $(test_lockfree_fifo_LIBS) -o $@ $^

%.o: %.c
	$(COMPILE) -o $@ $<

clean: 
	rm -f $(test_OBJECTS) $(test_lockfree_fifo_OBJECTS) $(EXECUTABLES) depend

depend:
	$(CC) -E -MM *.c > depend

include depend

.PHONY: all clean

.PRECIOUS: depend
