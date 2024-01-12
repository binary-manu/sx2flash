#ifndef DEV_FLASH_H
#define DEV_FLASH_H

#include <stdint.h>

#ifndef FLASH_PAGE_SIZE
#  define FLASH_PAGE_SIZE 128
#elif FLASH_PAGE_SIZE <= 0 || FLASH_PAGE_SIZE > 4294967296
# error FLASH_PAGE_SIZE must be positive and not greater than 2**32
#endif

#ifndef FLASH_SIZE
#  define FLASH_SIZE 131072 // Bytes
#elif FLASH_SIZE <= 0 || FLASH_SIZE > 4294967296
# error FLASH_SIZE must be positive and not greater than 2**32
#endif

#if FLASH_SIZE % FLASH_PAGE_SIZE != 0
# error FLASH_SIZE must be a multiple of FLASH_PAGE_SIZE
#endif

// We fix all data types to uint64_t since we want to emulate a flash of up to 4GB.
// This is required to test address extension records in the IHEX format.
// If the chosen flash size is smaller, everything will still work.
// We need 64 bits because the flash size itself is not representable using 32 bits

// Each of the following types must be able to hold all the intended
// address or page number/offset values PLUS ONE. That is, if the largest address
// is 65535, flash_address_t must be able to store 65536. And so on for the others.

// An unsigned integer type large enough to hold an address of program store
// memory. Each address points to a byte.
typedef uint64_t flash_address_t;
// An unsigned integer type large enough to hold a page number
typedef uint64_t flash_page_num_t;
// An unsigned integer type large enough to hold a page offset
typedef uint64_t flash_offset_t;

#endif  // DEV_FLASH_H