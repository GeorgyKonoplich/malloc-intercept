#include "tracing.h"

#include <cstring>
#include <unistd.h>

void ghoard::print_object(char const* message) {
    ::write(2, message, strlen(message));
}

void ghoard::print_object(void* px) {
    char const* hexdigits = "0123456789abcdef";

    char buffer[32];

    size_t n = (size_t) px;
    size_t divisor = std::numeric_limits<size_t>::max() / 2;

    char* p = buffer;
    *p++ = '0';
    *p++ = 'x';

    do {
        *p++ = hexdigits[(n / divisor) % 16];
        divisor /= 16;
    }    while (divisor != 0);

    *p = '\0';

    print_object(buffer);
}

void ghoard::print_object(size_t n) {
    char buffer[32];

    size_t divisor = 1;

    while (divisor <= (n / 10))
        divisor *= 10;

    char* p = buffer;
    do {
        *p++ = ((n / divisor) % 10) + '0';
        divisor /= 10;
    }    while (divisor != 0);

    *p = '\0';

    print_object(buffer);
}

void ghoard::print() {
}

bool ghoard::trace_enabled() {
    static char * GHOARD_TRACE = getenv("GHOARD_TRACE");
    static bool enabled = GHOARD_TRACE != NULL && GHOARD_TRACE[0] == '1';
    return enabled;
}

