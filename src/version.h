#ifndef SDM_VERSION_H
#define SDM_VERSION_H

#define SDM_VERSION_MAJOR 1
#define SDM_VERSION_MINOR 0
#define SDM_VERSION_MICRO 0
#define SDM_VERSION_HEX ((SDM_VERSION_MAJOR << 16) | (SDM_VERSION_MINOR << 8) | (SDM_VERSION_MICRO))

/* https://gcc.gnu.org/onlinedocs/cpp/Stringification.html */
#define XSTR(x) STR(x)
#define STR(x) #x

#define SDM_VERSION_STR "v" XSTR(SDM_VERSION_MAJOR) "." XSTR(SDM_VERSION_MINOR) "." XSTR(SDM_VERSION_MICRO)

#endif
