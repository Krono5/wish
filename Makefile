.PHONY: clean

wish: clean main.o
	gcc -o wish main.o

main.o: main.c
	gcc -c main.c

run: main.o
	./wish

clean:
	rm -f $(OBJS) $(OUT)

test: wish
	./test-wish.sh
