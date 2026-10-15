#include "platform/psx_probe.h"

#include "p8/fatal.h"
#include "platform/psx_fine_clock.h"
#include "platform/psx_tty.h"

#include <stdio.h>

enum { CALIBRATION_HBLANKS = 64, DELAY_MAX_HBLANKS = 200 };

static uint32_t g_calibration;
static uint32_t g_delay_hblanks;
static uint32_t g_delay_ticks;
static uint32_t g_delay_calls;

void psx_probe_start(void) {
    psx_fine_clock_start();
    g_calibration = psx_fine_clock_calibrate();
}

static void check_delay(uint32_t hblanks) {
    char message[96];
    if (g_calibration == 0) p8_fatal("psx_probe_delay: psx_probe_start was not called");
    if (hblanks == 0 || hblanks > DELAY_MAX_HBLANKS) {
        snprintf(message, sizeof message, "psx_probe_delay: %lu hblanks outside 1..%d (fine clock wrap)",
                 (unsigned long)hblanks, DELAY_MAX_HBLANKS);
        p8_fatal(message);
    }
    if (g_delay_calls != 0 && hblanks != g_delay_hblanks) {
        snprintf(message, sizeof message, "psx_probe_delay: %lu hblanks after %lu: one delay value per probe build",
                 (unsigned long)hblanks, (unsigned long)g_delay_hblanks);
        p8_fatal(message);
    }
}

void psx_probe_delay(uint32_t hblanks) {
    if (g_delay_calls == 0 || hblanks != g_delay_hblanks) {
        check_delay(hblanks);
        g_delay_hblanks = hblanks;
        g_delay_ticks = (hblanks * g_calibration + CALIBRATION_HBLANKS / 2) / CALIBRATION_HBLANKS;
    }
    g_delay_calls++;
    uint16_t start = psx_fine_clock_read();
    while ((uint16_t)(psx_fine_clock_read() - start) < g_delay_ticks) {
    }
}

void psx_probe_write(void) {
    psx_tty_printf("psx-probe: clk8-per-64-hblanks %lu\n", (unsigned long)g_calibration);
    if (g_delay_calls == 0) return;
    psx_tty_printf("psx-probe: delay %lu calls %lu\n", (unsigned long)g_delay_hblanks, (unsigned long)g_delay_calls);
}
