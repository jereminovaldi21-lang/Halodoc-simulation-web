CONFIG_FOLDER ?= config/
APP = halodoc
BIN = bin
SRC = src

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Wpedantic

SRCS = $(shell find src -name "*.c")
OBJS = $(SRCS:.c=.o)

all: $(BIN)/$(APP)

$(BIN)/$(APP): $(SRCS) header/halodoc.h
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$(APP) $(SRCS)

build: $(SRCS) header/halodoc.h
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/$(APP) $(SRCS)

run: $(BIN)/$(APP)
	./$(BIN)/$(APP) $(CONFIG_FOLDER)

clean:
	rm -rf $(BIN)

.PHONY: all run clean
