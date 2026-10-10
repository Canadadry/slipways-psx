#ifndef DEVTOOLS_RECORD_START_H
#define DEVTOOLS_RECORD_START_H

#include "game/input_step.h"

#include <stddef.h>
#include <stdint.h>

/* Reads one line of hex digit pairs, then a newline and nothing else, into bytes (capacity of at
   least SAVE_STATE_MAX_BYTES). Returns the byte count, or 0 with "<path>: <reason>" in error,
   also when the bytes do not decode. */
int record_start_read_hex(const char *path, uint8_t *bytes, int capacity, char *error, size_t error_size);

/* At the top of a frame only: right after boot's cart_init, or after at least one cart_update60
   since the last record_start_return. Resumes the record, then re-inits step on the default
   table. Returns 0 and touches nothing when the bytes do not decode. */
int record_start_resume(input_step_t *step, const uint8_t *bytes, int count);

/* Between frames only: back to the title, then step re-inited on the compiled table. */
void record_start_return(input_step_t *step);

#endif
