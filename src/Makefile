CC=clang
CFLAGS=-g

ODIR=obj

_OBJS = args.o clients.o handlers.o http.o main.o str_util.o log.o mimetype.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

webserver: $(ODIR) $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

$(ODIR):
	mkdir $(ODIR)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o webserver
