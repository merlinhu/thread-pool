pool : thread-pool.o main.o
    gcc -o pool thread-pool.o main.o
main.o : main.c
    gcc -c main.c
thread-pool.o : thread-pool.c thread-pool.h
    gcc -c thread-pool.c
clean:
    rm pool main.o thread.o