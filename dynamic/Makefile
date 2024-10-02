all: library library2 app

library:
	gcc -I. -shared -fPIC dynamic.c -o libdynamic.so

library2:
	gcc -I. -shared -fPIC dynamic2.c -o libdynamic2.so

app:
	gcc -I. -L. main.c -Wl,-rpath=. -ldynamic

static:
	gcc -I. -L. main.c -Wl,-rpath=. -ldynamic

clean:
	rm a.out libdynamic*
