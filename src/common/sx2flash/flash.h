#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>

#include <sx2flash/device/flash.h>

#define FLASH_INVALID_PAGE_NUM ((flash_page_num_t)-1)

// Write the contents of data to the PS page addressesd by page.
// An erase cycle is performed automatically. data must point to a buffer
// large at least FLASH_PAGE_SIZE bytes.
// Will return true on success and false on error
bool flash_write(flash_page_num_t page, const uint8_t *data);
// Read the contents of the PS page addressesd by page into data
// data must point to a buffer large at least FLASH_PAGE_SIZE bytes.
// Will return true on success and false on error
bool flash_read(flash_page_num_t page, uint8_t *data);
// Splits a PS address a into page number and offset and writes them to pn and off.
// pn will be set to FLASH_INVALID_PAGE_NUM if the address is outside the
// PS address space for the frvice.
void flash_map_address_to_page(flash_address_t a, flash_page_num_t *pn, flash_offset_t *off);

#endif // FLASH_H