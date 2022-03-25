CC = gcc
CFLAGS = -g -Wextra -Werror -Wall -Wvla -std=c99

test: tests.o test_runner.o Node.o HashSet.o
	$(CC) $(CFLAGS) tests.o test_runner.o Node.o HashSet.o -o ex4_tests
Node.o: ../itai/Node.c Node.h
	$(CC) -c Node.c
HashSet.o: ../itai/HashSet.c HashSet.h
	$(CC) -c HashSet.c
tests.o: ../itai/tests.c test_runner.h
	$(CC) -c tests.c
test_runner.o: ../itai/test_runner.h test_runner.c
	$(CC) -c test_runner.c
run_tests:
	make test && ./ex4_tests
run_valgrind:
	make test && valgrind --leak-check=full ./ex4_tests