entabmake: entab.c
	gcc -o entab entab.c -I/usr/local/opt/argp-standalone/include -L/usr/local/opt/argp-standalone/lib -largp
