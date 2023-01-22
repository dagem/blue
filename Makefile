FLAGS=-O0 --std=c99 -Wall --pedantic

blue: src/main.c src/editor.c src/syntax.c 
	gcc $^ ${FLAGS} -o $@

clean:
	rm -f blue
