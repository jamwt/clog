clog: clog.c
	gcc -O2 -Wall -Werror -g clog.c -o clog

clean:
	rm -f clog

install:
	cp clog /usr/local/bin/clog
