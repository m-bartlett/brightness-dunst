# Compile each .c file into its own synonymous executable
TARGET = brightness-dunst

CC = gcc
PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin
LDFLAGS += -lm

.PHONY:
	install

all: $(TARGET)

$(TARGET): $(patsubst %,%.c, $(TARGET))
	$(CC) $(LDFLAGS) $< -o $@

install: $(TARGET)
# 4 is SetUID sticky bit
	install --owner root --mode 4755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm "$(BINPREFIX)/$(TARGET)"

clean:
	rm $(TARGET)