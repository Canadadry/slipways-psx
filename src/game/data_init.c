#include "game/data_init.h"

#include "pico8_research.h"
#include "pico8_tech_gate.h"

void pico8_data_init(void) {
    /* hp must be ready before build_gb builds one. */
    pico8_research_init();
    pico8_research_build_gb();

    pico8_nb_e_reset();
}

void pico8_data_shutdown(void) {
    pico8_research_shutdown();
}
