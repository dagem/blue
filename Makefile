FLAGS=-O0 --std=c99 -Wall --pedantic

blue: src/main.c src/editor.c 
	gcc $^ ${FLAGS} -o $@

clean:
	rm -f blue
