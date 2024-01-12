#ifndef IHEX_H
#define IHEX_H

#include <stddef.h>
#include <stdint.h>

#ifndef IHEX_MAX_DATA_PER_RECORD 
#  define IHEX_MAX_DATA_PER_RECORD 16 // Assume the longest record carries 16 bytes
#elif IHEX_MAX_DATA_PER_RECORD <= 0 || IHEX_MAX_DATA_PER_RECORD > 255
#  error Invalid IHEX_MAX_DATA_PER_RECORD: must be in range [0,255]
#endif

#define IHEX_RECORD_SIZE (1 + 2 * (1 + 2 + 1 + IHEX_MAX_DATA_PER_RECORD + 1))
#define IHEX_MIMIMUM_BYTES_TO_LENGTH 2
#define IHEX_MARKER ':'

typedef enum ihex_record_status {
    IHEX_RECORD_STATUS_OK,
    IHEX_RECORD_STATUS_PARSE_ERROR,
    IHEX_RECORD_STATUS_END,
} ihex_record_status_t;

typedef enum ihex_record_type {
    IHEX_RECORD_TYPE_DATA               = 0,
    IHEX_RECORD_TYPE_END                = 1,
    IHEX_RECORD_TYPE_HIGHADDR_SEGMENTED = 2,
    IHEX_RECORD_TYPE_HIGHADDR_LINEAR    = 4,
} ihex_record_type_t;

ihex_record_status_t ihex_process_record(const uint8_t *data, size_t *sz);

#endif // IHEX_H