#ifndef DEVTOOLS_PAD_CAPTION_H
#define DEVTOOLS_PAD_CAPTION_H

#include <stddef.h>

#define PAD_CAPTION_LINE_CHARS 32
#define PAD_CAPTION_LINES_MAX 2
#define PAD_CAPTION_SHOWN_MIN 150
#define PAD_CAPTION_QUIET_MIN 90
#define PAD_CAPTION_TEXT_SIZE (PAD_CAPTION_LINES_MAX * (PAD_CAPTION_LINE_CHARS + 1))

/* 1 and the checked text in `out`, lines joined by '\n'; or 0 and the reason, without a location, in `error`. */
int pad_caption_parse(const char *script_text, char *out, size_t out_size, char *error, size_t error_size);

typedef struct {
    int from; /* frame indexes of the output: shown on from <= i < to */
    int to;
    char text[PAD_CAPTION_TEXT_SIZE];
} pad_caption_t;

typedef struct {
    pad_caption_t *captions;
    int capacity;
    int count;
    int shown; /* 1 while captions[count - 1] has no `to` yet */
} pad_caption_track_t;

/* `storage` stays owned by the caller and must outlive the track. */
void pad_caption_track_init(pad_caption_track_t *track, pad_caption_t *storage, int capacity);
/* show, clear and end close the shown caption at `frame`: 0 and the reason in `error` if it was shown under
 * PAD_CAPTION_SHOWN_MIN frames (unless `checked` is 0), which leaves it shown. clear also fails with nothing shown. */
int pad_caption_track_show(pad_caption_track_t *track, int frame, const char *text, char *error, size_t error_size);
int pad_caption_track_clear(pad_caption_track_t *track, int frame, char *error, size_t error_size);
/* 0 for a button down fewer than PAD_CAPTION_QUIET_MIN frames after the shown caption appeared. */
int pad_caption_track_frame(const pad_caption_track_t *track, int frame, int buttons, char *error, size_t error_size);
int pad_caption_track_end(pad_caption_track_t *track, int frame, int checked, char *error, size_t error_size);
/* The bare "captions" member for p8_input_save's header; its length, or 0 and "" when there is no caption. */
int pad_caption_write_member(const pad_caption_track_t *track, char *out, size_t out_size);

#endif
