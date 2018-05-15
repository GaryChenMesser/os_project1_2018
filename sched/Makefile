CC      = gcc
CFLAGS  = -Wall -pthread
OBJECT  = main_v3.o mysched.o
EXEC    = sched.out

default : $(EXEC)

$(EXEC) : $(OBJECT) 
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECT)
main_v3.o : main_v3.c mysched.h
	$(CC) $(CFLAGS) -c main_v3.c
mysched.o : mysched.c mysched.h
	$(CC) $(CFLAGS) -c mysched.c

clean :
	$(RM) $(EXEC) *.o *.out
