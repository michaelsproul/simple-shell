DEPS=shell.o lexer.o parse.o exec.o dynamic_array.o util.o

all: shell

shell: $(DEPS)
	$(CC) $(CFLAGS) $(DEPS) -o shell

# Flex lexer.
lexer.o: lexer.c
	$(CC) -w $< -c -o $@

lexer.c: lexer.y
	flex -o$@ $<

clean:
	rm -rf shell *.o lexer.c

.PHONY: all clean
