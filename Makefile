SOURCES=myShell.cpp split.cpp
OBJS=$(patsubst %.cpp, %.o, $(SOURCES))
CPPFLAGS=-ggdb3 -Wall -Werror -pedantic -std=gnu++98

compress: $(OBJS)
	g++ $(CPPFLAGS) -o myShell $(OBJS)
%.o: %.cpp split.h
	g++ $(CPPFLAGS) -c $<

clean:
	rm compress *~ *.o