#include <sx2flash/ihex.h>

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <sx2flash/util.h>
#include <sx2flash/flash.h>

typedef struct cksum_context {
    uint8_t slot;
    uint8_t cksum;
    struct slot {
        const uint8_t *data;
        size_t size;
    } slots[2];
} cksum_context_t;

static inline struct slot* cksum_ctx_slot(cksum_context_t *ctx) {
    return ctx->slots + ctx->slot;
}

static void cksum_ctx_init(cksum_context_t *ctx, const uint8_t *data0, size_t sz0, const uint8_t *data1, size_t sz1) {
    *ctx = (cksum_context_t){
        .slots = {
            { .data = data0, .size = sz0 },
            { .data = data1, .size = sz1 },
        },
        .cksum = 0,
        .slot = !sz0,
   };
}

static inline void cksum_ctx_copy(cksum_context_t *new_ctx, const cksum_context_t *old_ctx) {
   *new_ctx = *old_ctx;
}

// Address used to extend the per-record address beyond 64k
static uint32_t high_address = 0;
// Buffer for caching a partial record, used to handle records broken
// among packets
static uint8_t partial_record[IHEX_RECORD_SIZE];
static size_t partial_size = 0;
// Combines records that go into the same page
static uint8_t flash_page[FLASH_PAGE_SIZE];
// Page number the buffer should be written to
flash_page_num_t flash_page_num = FLASH_INVALID_PAGE_NUM;
// If the page buffer is dirty
bool flash_page_writeback = false;

static int parse_hex(uint8_t msb, uint8_t lsb) {
    int ret = -1;
    when_false(isxdigit(msb), done);
    when_false(isxdigit(lsb), done);
    ret = isdigit(msb) ? msb - '0' : tolower(msb) - 'a' + 10;
    ret *= 16;
    ret += isdigit(lsb) ? lsb - '0' : tolower(lsb) - 'a' + 10;

done:
    return ret;
}

static void skip_char(cksum_context_t *ctx) {
    struct slot *slot = cksum_ctx_slot(ctx);
    if (slot->size >= 1) {
        slot->data++;
        slot->size--;
    }
    if (slot->size == 0 && !ctx->slot) {
        ctx->slot = 1;
    }
}

static inline size_t remaining_data(cksum_context_t *ctx) {
    return ctx->slots[0].size + ctx->slots[1].size;
}

static int peek_char(cksum_context_t *ctx) {
    int ret = -1;

    when_false(remaining_data(ctx) >=1, done);
    ret = *cksum_ctx_slot(ctx)->data;

done:
    return ret;
}

#define peek_char_checked(ctx, where) ({ int b = peek_char((ctx)); when_true(b < 0, where); b; })

static int next_byte(cksum_context_t *ctx) {
    int ret = -1;

    when_false(remaining_data(ctx) >=2, done);
    int msb = peek_char(ctx);
    skip_char(ctx);
    int lsb = peek_char(ctx);
    skip_char(ctx);
    int b = parse_hex(msb, lsb);
    when_false(b >= 0, done);
    ctx->cksum += b;
    ret = b;

done:
    return ret;
}

#define next_byte_checked(ctx, where) ({ int b = next_byte((ctx)); when_true(b < 0, where); b; })

static bool ihex_writeback(void) {
    if (!flash_page_writeback) {
        return true;
    }
    when_false(flash_write(flash_page_num, flash_page), err);
    flash_page_writeback = false;
    return true;

err:
    return false;
}

static bool ihex_handle_data(cksum_context_t *ctx, size_t data_len, uint16_t address) {
    flash_page_num_t page_num;
    flash_offset_t page_off;
    flash_address_t full_address = high_address + address;
    // flash_address_t does not necessarily represent a full uint32_t that IHEX can convey
    when_false(full_address == high_address + address, err);
    flash_map_address_to_page(full_address, &page_num, &page_off);

    // flash_page_num != page switch, need writeback and load new
    // page_num == FLASH_INVALID_PAGE_NUM -> trigger error
    if (page_num == FLASH_INVALID_PAGE_NUM || flash_page_num != page_num) {
        when_false(ihex_writeback(), err);
        when_false(flash_read(page_num, flash_page), err);
        flash_page_num = page_num;
    }

    // From here, we are sure page_num is within the valid flash address space
    // and flash_page_num equals it.
    while (data_len > 0) {
        // Fill current page
        uint8_t *dst = flash_page + page_off;
        flash_offset_t xfer = minimum(data_len, FLASH_PAGE_SIZE - page_off);
        for (; xfer > 0; xfer--) {
            // This unchecked call is OK: the caller has already verified the cksum,
            // so we are sure there's a valid byte next
            *dst = next_byte(ctx);
            dst++;
            data_len--;
        }
        flash_page_writeback = true;
        if (data_len == 0) {
            break;
        }
        when_false(ihex_writeback(), err);
        flash_page_num++;
        page_off = 0;
        when_false(flash_read(flash_page_num, flash_page), err);
    }

    return true;

err:
    return false;
}

ihex_record_status_t ihex_process_record(const uint8_t * const data, size_t * const sz) {
    ihex_record_status_t ret = IHEX_RECORD_STATUS_OK;
    cksum_context_t ctx;
    cksum_ctx_init(&ctx, partial_record, partial_size, data, *sz);

    // Skip blanks between records
    while (isspace(peek_char(&ctx))) {
        skip_char(&ctx);
    }
    if (!remaining_data(&ctx)) {
        goto done;
    }

    cksum_context_t head_ctx;
    cksum_ctx_copy(&head_ctx, &ctx);
    when_false(peek_char(&ctx) == IHEX_MARKER, parse_error);
    skip_char(&ctx);
    when_false(remaining_data(&ctx) >= IHEX_MIMIMUM_BYTES_TO_LENGTH, handle_partial_record);

    int data_len = next_byte_checked(&ctx, parse_error);
    when_true(data_len * 2 + 8 > remaining_data(&ctx), handle_partial_record);

    uint16_t address = next_byte_checked(&ctx, parse_error);
    address = (address << 8) | next_byte_checked(&ctx, parse_error);

    ihex_record_type_t type = next_byte_checked(&ctx, parse_error);

    // Verify the checksum before committing any action
    cksum_context_t tmp_ctx;
    cksum_ctx_copy(&tmp_ctx, &ctx);
    for (size_t i = data_len; i > 0; i--) {
        next_byte_checked(&tmp_ctx, parse_error);
    }
    // Add the checksum too, the result must be zero
    next_byte_checked(&tmp_ctx, parse_error);
    when_false(tmp_ctx.cksum == 0, parse_error);

    switch (type) {
        case IHEX_RECORD_TYPE_DATA:
            // This call must consume all data bytes on success
            when_false(ihex_handle_data(&ctx, data_len, address), parse_error);
            break;
        case IHEX_RECORD_TYPE_END:
            when_false(data_len == 0, parse_error);
            when_false(ihex_writeback(), parse_error);
            ret = IHEX_RECORD_STATUS_END;
            break;
        case IHEX_RECORD_TYPE_HIGHADDR_SEGMENTED: // fallthrough
        case IHEX_RECORD_TYPE_HIGHADDR_LINEAR: {
            when_false(data_len == 2, parse_error);
            const uint32_t start_h = next_byte(&ctx);
            const uint32_t start_l = next_byte(&ctx);
            if (type == IHEX_RECORD_TYPE_HIGHADDR_LINEAR) {
                high_address = (start_h << 24) | (start_l << 16);
            } else {
                high_address = (start_h << 12) | (start_l << 4);
            }
            break;
        }
        default:
            // Unrecognized records are ignored
            // In particular we don't care about entry point
            // addresses as the reset vector is fixed
            cksum_ctx_copy(&ctx, &tmp_ctx);
            // In this case the checksum was already parsed from tmp_ctx
            // so skip the first line below
            goto dont_swallow_ck;
    }

done:
    // Swallow the checksum
    next_byte(&ctx);
dont_swallow_ck:
    *sz -= ctx.slots[1].size;
    memmove(partial_record, ctx.slots[0].data, ctx.slots[0].size);
    partial_size = ctx.slots[0].size;
    return ret;

handle_partial_record:
    if (partial_size == sizeof partial_record) {
        // if we have a full buffer and still partial, the record is too large to handle
        goto parse_error;
    }
    size_t add = minimum(sizeof partial_record - partial_size, head_ctx.slots[1].size);
    memmove(partial_record + partial_size, head_ctx.slots[1].data, add);
    partial_size += add;
    // NOTE: we must also count the initial blanks skipped from the record data in slots[1].
    // slots[0] points to the partial_record and it never contains leading blanks.
    *sz = add + head_ctx.slots[1].data - data;
    return IHEX_RECORD_STATUS_OK;

parse_error:
    return IHEX_RECORD_STATUS_PARSE_ERROR;
}