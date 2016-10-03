CC = g++-5
FLAGS = -std=c++11

install: Makefile setport runtest
	echo alias setport=$$(find `pwd` -samefile ./setport) >>~/.bash_aliases
	
setport: Makefile setport.cpp
	$(CC) $(FLAGS) -o setport setport.cpp

test: Makefile setport test.cpp
	$(CC) $(FLAGS) -o test test.cpp

runtest: Makefile test setport
	./test en
	./test es

clean:
	if test -f setport; then rm setport; fi
	if test -f test; then rm test; fi