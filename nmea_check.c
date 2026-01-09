#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_digit(char digit) {
    if (digit >= '0' && digit <= '9') return digit - '0';
    digit = (char)toupper((unsigned char)digit);
    return digit >= 'A' && digit <= 'F' ? digit - 'A' + 10 : -1;
}

static int coordinate(const char *field, const char *hemisphere,
                      int latitude, double *degrees) {
    char *end;
    errno = 0;
    double encoded = strtod(field, &end);
    if (errno || end == field || *end || !isfinite(encoded) || encoded < 0)
        return 0;
    double whole = floor(encoded / 100.0);
    double minutes = encoded - whole * 100.0;
    double limit = latitude ? 90.0 : 180.0;
    if (minutes >= 60 || whole > limit || (whole == limit && minutes != 0))
        return 0;
    if (strlen(hemisphere) != 1) return 0;
    char direction = hemisphere[0];
    if (latitude ? direction != 'N' && direction != 'S'
                 : direction != 'E' && direction != 'W') return 0;
    *degrees = (whole + minutes / 60) *
               (direction == 'S' || direction == 'W' ? -1 : 1);
    return 1;
}

static int check_sentence(char *sentence) {
    char *star = strchr(sentence, '*');
    if (sentence[0] != '$' || !star || strlen(star + 1) != 2) return 0;
    int high = hex_digit(star[1]), low = hex_digit(star[2]);
    if (high < 0 || low < 0) return 0;
    unsigned checksum = 0;
    for (char *cursor = sentence + 1; cursor < star; ++cursor)
        checksum ^= (unsigned char)*cursor;
    if (checksum != (unsigned)(high * 16 + low)) return 0;
    *star = '\0';
    char *fields[24];
    size_t count = 1;
    fields[0] = sentence + 1;
    for (char *cursor = fields[0]; *cursor; ++cursor) {
        if (*cursor == ',') {
            if (count == 24) return 0;
            *cursor = '\0';
            fields[count++] = cursor + 1;
        }
    }
    int offset;
    if (!strcmp(fields[0], "GPGGA") || !strcmp(fields[0], "GNGGA")) {
        if (count < 7 || !strcmp(fields[6], "0") || !*fields[6]) return 0;
        offset = 2;
    } else if (!strcmp(fields[0], "GPRMC") || !strcmp(fields[0], "GNRMC")) {
        if (count < 7 || strcmp(fields[2], "A")) return 0;
        offset = 3;
    } else return 0;
    double latitude, longitude;
    if (!coordinate(fields[offset], fields[offset + 1], 1, &latitude) ||
        !coordinate(fields[offset + 2], fields[offset + 3], 0, &longitude))
        return 0;
    printf("%s,%.7f,%.7f,%02X\n", fields[0], latitude, longitude, checksum);
    return 1;
}

int main(void) {
    char sentence[256];
    unsigned line = 0;
    int failed = 0;
    puts("sentence,latitude_deg,longitude_deg,checksum");
    while (fgets(sentence, sizeof sentence, stdin)) {
        ++line;
        size_t length = strcspn(sentence, "\r\n");
        if (length == sizeof sentence - 1 && sentence[length - 1] != '\n') {
            int next;
            while ((next = getchar()) != '\n' && next != EOF) {}
            fprintf(stderr, "line %u: sentence too long\n", line);
            failed = 1;
            continue;
        }
        sentence[length] = '\0';
        if (!check_sentence(sentence)) {
            fprintf(stderr, "line %u: invalid sentence or unavailable fix\n", line);
            failed = 1;
        }
    }
    return ferror(stdin) || failed;
}
