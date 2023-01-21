FLAGS=-O0 --std=c99 -Wall --pedantic

blue: main.c
	gcc $^ ${FLAGS} -o $@

clean:
	rm -f blue
