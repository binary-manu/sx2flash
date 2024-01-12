#include <sx2flash/flash.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <sx2flash/util.h>

#define ME "native flash"

#define FLASHEND (FLASH_SIZE - 1)

static int flash_fd = -1;

void __attribute__((constructor)) open_flash_file(void) {
    flash_fd = open("flash.bin", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    when_true(flash_fd < 0, quit);
    when_true(ftruncate(flash_fd, FLASHEND + 1ul) < 0, quit);
    return;

quit:
    perror(ME " init");
    exit(EXIT_FAILURE);
}

bool flash_write(flash_page_num_t page, const uint8_t *data) {
    when_true(page > FLASHEND / FLASH_PAGE_SIZE, quit);
    flash_address_t addr = page * FLASH_PAGE_SIZE;
    when_true(pwrite(flash_fd, data, FLASH_PAGE_SIZE, addr) < 0, quit);
    return true;

quit:
    return false;
}

bool flash_read(flash_page_num_t page, uint8_t *data) {
    when_true(page > FLASHEND / FLASH_PAGE_SIZE, quit);
    flash_address_t addr = page * FLASH_PAGE_SIZE;
    when_true(pread(flash_fd, data, FLASH_PAGE_SIZE, addr) < 0, quit);
    return true;

quit:
    return false;
}

void flash_map_address_to_page(flash_address_t a, flash_page_num_t *pn, flash_offset_t *off) {
    if (a > FLASHEND) {
        *pn = FLASH_INVALID_PAGE_NUM;
        return;
    }
    *pn = a / FLASH_PAGE_SIZE;
    *off = a % FLASH_PAGE_SIZE;
}