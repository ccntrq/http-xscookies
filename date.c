#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "date.h"

/* length of a string like "Sun, 08-Jan-2006 13:56:17 GMT"]" */
#define DATE_FORMAT_LEN 29

double date_compute(const char *date)
{
    /* special case when date is the string "now" */
    if (date[0] == 'n' &&
        date[1] == 'o' &&
        date[2] == 'w' &&
        date[3] == '\0') {
        return time(0);
    }

    int state = 0;
    int negative = -1;
    int part[2] = {0,0};
    int p = 0;
    double decimals = 1;
    char term = 's';
    for (int e = 0; date[e] != '\0'; ++e) {
        char c = date[e];
        if (isspace(c)) {
            if (state > 0) {
                return 0;
            }
            continue;
        } else if (c == '+') {
            if (state >= 1) {
                return 0;
            }
            state = 1;
            negative = 0;
        } else if (c == '-') {
            if (state >= 1) {
                return 0;
            }
            state = 1;
            negative = 1;
        } else if (isdigit(c)) {
            if (state > 2) {
                return 0;
            }
            state = 2;
            part[p] = 10 * part[p] + c - '0';
            if (p > 0) {
                decimals *= 10;
            }
        } else if (c == '.') {
            if (state > 2) {
                return 0;
            }
            if (p >= 1) {
                return 0;
            }
            state = 2;
            ++p;
        } else if (c == 'y' ||
                   c == 'M' ||
                   c == 'd' ||
                   c == 'h' ||
                   c == 'm' ||
                   c == 's') {
            if (state >= 3) {
                return 0;
            }
            state = 3;
            term = c;
        } else {
            return 0;
        }
    }

    /* We require at least a number */
    if (state < 2) {
        return 0;
    }

    double offset = (double) part[0];

    /* digits only => epoch */
    if (state == 2 && negative < 0) {
        return offset;
    }

    offset += (double) part[1] / decimals;
    if (negative == 1) {
        offset = - offset;
    }
    switch (term) {
        case 'y':
            offset *= 86400 * 365;
            break;

        case 'M':
            offset *= 86400 * 30;
            break;

        case 'd':
            offset *= 86400;
            break;

        case 'h':
            offset *= 3600;
            break;

        case 'm':
            offset *= 60;
            break;

        case 's':
        default:
            break;
    }
    return time(0) + offset;
}

Buffer* date_format(double date, Buffer* format)
{
    static const char* Mon[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec",
    };
    static const char* Day[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat",
    };

    time_t t = (time_t) date;
    struct tm gmt;
    gmtime_r(&t, &gmt);

    buffer_ensure_delta(format, DATE_FORMAT_LEN);
    sprintf(format->data,
            "%3s, %02d-%3s-%04d %02d:%02d:%02d %3s",
            Day[gmt.tm_wday % 7],
            gmt.tm_mday,
            Mon[gmt.tm_mon % 12],
            gmt.tm_year + 1900,
            gmt.tm_hour,
            gmt.tm_min,
            gmt.tm_sec,
            "GMT");
    format->pos = DATE_FORMAT_LEN;
    return format;
}
