# info project
VERSION = "0.4.0"
EXE = tasker

# compile
CC = gcc
CFLAGS = -std=gnu11 -Wall -Werror -DVERSION=\"$(VERSION)\"
DFLAGS = -g -DDEBUG
LIBS = -lm -lncurses

# directory
SRCDIR = src/
OBJDIR = build/
DEBDIR = debug/
EXEDIR = /usr/local/bin/

# source files
SRC = commands.c graphics.c utils.c main.c
OBJ = ${SRC:.c=.o}
OBJD = ${patsubst %, $(OBJDIR)%, $(OBJ)}
DEB = ${patsubst %, $(DEBDIR)%, $(OBJ)}
HEAD = src/tasker.h

all: dirs tasker

$(OBJDIR)%.o: $(SRCDIR)%.c $(HEAD)
	$(CC) -c -o $@ $< $(CFLAGS)
	@echo "compile $@ success"

$(DEBDIR)%.o: $(SRCDIR)%.c $(HEAD)
	$(CC) -c -o $@ $< $(CFLAGS) $(DFLAGS)
	@echo "compile for debug $@ success"

dirs: 
	@mkdir -p $(OBJDIR)
	@echo "create directory $(OBJDIR)"

tasker: $(OBJD)
	$(CC) -o $(EXEDIR)$(EXE) $^ $(LIBS)
	@echo "build success"

debug: $(DEB)
	@mkdir -p $(DEBDIR)
	@echo "create directory $(DEBDIR)"
	$(CC) -o $(DEBDIR)$(EXE) $^ $(LIBS)

clean:
	@rm $(EXEDIR)$(EXE)
	@rm -rf $(OBJDIR)
	@echo "clean success"

