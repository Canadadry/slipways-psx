#include "game/data_init.h"

#include "game/research.h"
#include "game/tech_gate.h"

void data_init(void) {
    /* hp must be ready before build_gb builds one. */
    research_init();
    research_build_projects();

    tech_gate_reset_affordable_flags();
}

void data_init_data_shutdown(void) {
    research_shutdown();
}
