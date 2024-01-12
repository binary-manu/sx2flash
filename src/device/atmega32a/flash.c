#include <sx2flash/flash.h>

#include <avr/pgmspace.h>
#include <avr/io.h>

// Defined in spm.S
void spm_handler(uint8_t spmcr, uint16_t r1r0, uint16_t z);

bool flash_write(const flash_page_num_t page, const uint8_t *data) {
    if (page > FLASHEND / FLASH_PAGE_SIZE) {
        return false;
    }
    const flash_address_t addr = page * FLASH_PAGE_SIZE;
    spm_handler(_BV(PGERS) | _BV(SPMEN), 0, addr);
    for (flash_offset_t i = 0; i < FLASH_PAGE_SIZE; i += 2) {
        spm_handler(_BV(SPMEN), (data[i + 1] << 8) | data[i], i);
    }
    spm_handler(_BV(PGWRT)  | _BV(SPMEN), 0, addr);
    spm_handler(_BV(RWWSRE) | _BV(SPMEN), 0, 0);
    for (flash_offset_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        if (pgm_read_byte(addr + i) != data[i]) {
            return false;
        }
    }
    return true;
}

bool flash_read(flash_page_num_t page, uint8_t *data) {
    if (page > FLASHEND / FLASH_PAGE_SIZE) {
        return false;
    }
    flash_address_t addr = page * FLASH_PAGE_SIZE;
    for (flash_offset_t i = FLASH_PAGE_SIZE; i > 0; i--) {
        *data = pgm_read_byte(addr);
        data++;
        addr++;
    }
    return true;
}

void flash_map_address_to_page(flash_address_t a, flash_page_num_t *pn, flash_offset_t *off) {
    if (a > FLASHEND) {
        *pn = FLASH_INVALID_PAGE_NUM;
        return;
    }
    *pn = a / FLASH_PAGE_SIZE;
    *off = a % FLASH_PAGE_SIZE;
}
