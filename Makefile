FLAGS=-O0 --std=c99 -Wall --pedantic

blue: main.c
	gcc $^ ${FLAGS} -o $@ editor.c

clean:
	rm -f blue
