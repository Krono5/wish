.PHONY: clean

wish: clean main.o
	gcc -o wish wish.o

main.o: wish.c wish.h
	gcc -c wish.c wish.h

run: main.o
	./wish

clean:
	rm -f $(OBJS) $(OUT)

test: wish
	./test-wish.sh
