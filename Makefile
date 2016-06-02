CC=clang
CFLAGS=-Wall -Wextra -Werror -Ofast
LIBS=-lpthread

ODIR=obj
SDIR=src

_OBJ = pi.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

pi: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o pi
