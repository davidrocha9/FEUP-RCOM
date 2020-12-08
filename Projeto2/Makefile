CC = gcc
CFLAGS = -Wall
DEPS = url_parser.h socket_handler.h ftp_handler.h
OBJ = url_parser.o socket_handler.o ftp_handler.o
TARGETS = download

all: download

%.o: %.c $(DEPS) 
	$(CC) $(CFLAGS) -c -o $@ $<

download: $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm

.PHONY : clean

clean:
	@rm *.o $(TARGETS)
