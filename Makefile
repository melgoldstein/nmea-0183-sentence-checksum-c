CC = gcc
CFLAGS = -O2 -Wall -Wextra -Werror -std=c99
all: nmea_check
nmea_check: nmea_check.c
	$(CC) $(CFLAGS) -Iinclude nmea_check.c -lm -o $@
clean:
	rm -f nmea_check nmea_check.exe *.o
.PHONY: all clean

