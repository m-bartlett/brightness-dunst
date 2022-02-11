# Compile each .c file into its own synonymous executable
TARGET = brightness-dunst

CC      =  g++
CFLAGS  += -std=c++17 -I$(PREFIX)/include
CFLAGS  += -D_POSIX_C_SOURCE=200112L
CFLAGS  += $(shell pkg-config --cflags libnotify)
LDFLAGS += -L$(PREFIX)/lib

LIBS     = -lm
LIBS		 += $(shell pkg-config --libs libnotify)

PREFIX    ?= /usr/local
BINPREFIX  = $(PREFIX)/bin

.PHONY:
	install

all: $(TARGET)

$(TARGET): $(patsubst %,%.c, $(TARGET))
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@  $(LIBS)


install: $(TARGET)
# 4 is SetUID sticky bit
	install --owner root --mode 4755 -D --target-directory "$(BINPREFIX)" "$(TARGET)"

uninstall:
	rm "$(BINPREFIX)/$(TARGET)"

clean:
	rm $(TARGET)