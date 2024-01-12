#ifndef ATMEGA32A_FLASH_H
#define ATMEGA32A_FLASH_H

#include <stdint.h>
#include <avr/io.h>

#define FLASH_PAGE_SIZE SPM_PAGESIZE

// Each of the following types must be able to hold all the intended
// address or page number/offset values PLUS ONE. That is, if the largest address
// is 65535, flash_address_t must be able to store 65536. And so on for the others.

// An unsigned integer type large enough to hold an address of program store
// memory. Each address points to a byte.
typedef uint16_t flash_address_t;
// An unsigned integer type large enough to hold a page number
typedef uint16_t flash_page_num_t;
// An unsigned integer type large enough to hold a page offset
typedef uint8_t flash_offset_t;

#endif  // ATMEGA32A_FLASH_H