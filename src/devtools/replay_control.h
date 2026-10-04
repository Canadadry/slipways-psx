#ifndef DEVTOOLS_REPLAY_CONTROL_H
#define DEVTOOLS_REPLAY_CONTROL_H

#include <stddef.h>

#define REPLAY_CONTROL_LIVE_FPS 60

#define REPLAY_CONTROL_STATUS_MAX 64

typedef struct {
    int level;
} replay_control_t;

void replay_control_init(replay_control_t *control);

void replay_control_faster(replay_control_t *control);
void replay_control_slower(replay_control_t *control);

void replay_control_uncap(replay_control_t *control);

int replay_control_speed_multiplier(const replay_control_t *control);

int replay_control_target_fps(const replay_control_t *control, int replay_active);

void replay_control_format_status(const replay_control_t *control, int frames_played, int frames_total, char *out, size_t out_size);

#endif
