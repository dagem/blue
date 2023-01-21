FLAGS=-O0 --std=c99 -Wall --pedantic

blue: src/main.c
	gcc $^ ${FLAGS} -o $@ src/editor.c

clean:
	rm -f blue
