#include "game/data_init.h"

#include "game/research.h"
#include "game/tech_gate.h"

void data_init(void) {
    /* hp must be ready before build_gb builds one. */
    pico8_research_init();
    pico8_research_build_gb();

    pico8_nb_e_reset();
}

void data_init_data_shutdown(void) {
    pico8_research_shutdown();
}
