#include "game/map_tables.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

const pico8_r_entry_t pico8_r_table[PICO8_R_COUNT] = {
    {"f", 28}, {"o", 21}, {"b", 18}, {"p", 19}, {"g", 48}, {"t", 22}, {"l", 20}, {"w", 12}, {"k", 24}, {"\?", 36},
    {"a", 33}, {"e", 54}, {"h", 25},
};

const pico8_iz_entry_t pico8_iz_table[PICO8_IZ_COUNT] = {
    {"28", "food"}, {"21", "ore"}, {"18", "bots"}, {"19", "people"}, {"48", "goods"}, {"22", "tech"},
    {"20", "organics"}, {"12", "luxury"}, {"24", "science"}, {"54", "energy"}, {"25", "joy"},
};

const pico8_hc_entry_t pico8_hc_table[PICO8_HC_COUNT] = {
    {"f-", 4, {0, 1, 2, 3}, "f+"},
    {"f+", 4, {0, 2, 3, 4}, NULL},
    {"-", 3, {1, 2, 3}, "+"},
    {"+", 3, {2, 3, 4}, NULL},
    {"structure", 4, {0, 1, 3, 5}, "qh+"},
    {"qh+", 4, {0, 2, 5, 8}, NULL},
};

const pico8_fd_entry_t pico8_fd_table[PICO8_FD_COUNT] = {
    {.npos = 7, .px = "", .lr = "", .fj = 0, .ja = "structure", .i = "p\?", .o = "k", .il = "superhuman ai"},
    {.npos = 6, .px = "", .lr = "", .fj = 0, .ja = "f+", .i = "p", .o = "h"},
    {.npos = 6, .px = "", .lr = "", .fj = 0, .ja = "-", .i = "", .o = "e"},
    {.npos = 7, .px = "", .lr = "", .fj = 0, .ja = "f-", .i = "l", .o = "f", .il = "xenofoods"},
    {.npos = 6, .px = "", .lr = "", .fj = 0, .ja = "f-", .i = "", .o = "\?"},
    {.npos = 8, .px = "", .lr = "", .fj = 0, .ja = "f-", .i = "\?\?", .o = "w", .il_false = 1, .dm = "\?"},
    {.npos = 6, .px = "nanotech", .lr = "f", .fj = 8, .ja = "-", .i = "o", .o = "t"},
    {.npos = 6, .px = "bots", .lr = "f", .fj = 8, .ja = "+", .i = "ot", .o = "b"},
    {.npos = 6, .px = "goods", .lr = "f", .fj = 10, .ja = "+", .i = "ob", .o = "g"},
    {.npos = 6, .px = "gadgets", .lr = "f", .fj = 10, .ja = "+", .i = "ot", .o = "g"},
    {.npos = 8, .px = "hiveworld", .lr = "e", .fj = 5, .ja = "+", .i = "f", .o = "p", .il_false = 1, .dm = "g"},
    {.npos = 8, .px = "colony", .lr = "o", .fj = 10, .ja = "-", .i = "of", .o = "p", .il = "gene rewriting", .dm = "g"},
    {.npos = 8, .px = "colony", .lr = "x", .fj = 10, .ja = "-", .i = "tf", .o = "p", .il = "gene rewriting", .dm = "g"},
    {.npos = 6, .px = "mine", .lr = "m", .fj = 5, .ja = "-", .i = "p", .o = "o"},
    {.npos = 6, .px = "strip", .lr = "r", .fj = 5, .ja = "-", .i = "pf", .o = "o"},
    {.npos = 8, .px = "agroworld", .lr = "e", .fj = 5, .ja = "-", .i = "b", .o = "f", .il = "xenofoods", .dm = "l"},
    {.npos = 8, .px = "algae", .lr = "o", .fj = 8, .ja = "-", .i = "b", .o = "f", .il = "xenofoods", .dm = "l"},
    {.npos = 6, .px = "breed", .lr = "o", .fj = 5, .ja = "+", .i = "p", .o = "l"},
    {.npos = 6, .px = "breed", .lr = "x", .fj = 5, .ja = "-", .i = "b", .o = "l"},
    {.npos = 6, .px = "hunt", .lr = "j", .fj = 5, .ja = "-", .i = "p", .o = "l"},
    {.npos = 6, .px = "dismantle", .lr = "r", .fj = 5, .ja = "-", .i = "pf", .o = "t"},
    {.npos = 6, .px = "scavenge", .lr = "r", .fj = 5, .ja = "-", .i = "pf", .o = "b"},
    {.npos = 6, .px = "tourism", .lr = "j", .fj = 10, .ja = "f-", .i = "pg", .o = "w"},
    {.npos = 6, .px = "luxury", .lr = "e", .fj = 10, .ja = "f-", .i = "tg", .o = "w"},
    {.header = "xenofoods"},
    {.npos = 8, .px = "farm", .lr = "xj", .fj = 10, .ja = "-", .i = "b", .o = "f", .il_false = 1, .dm = "l"},
    {.header = "geneseeds"},
    {.npos = 8, .px = "habitat", .lr = "si", .fj = 12, .ja = "-", .i = "lf", .o = "p", .il_false = 1, .dm = "g"},
    {.npos = 6, .px = "genefarm", .lr = "si", .fj = 12, .ja = "-", .i = "lb", .o = "f"},
    {.header = "drillbots"},
    {.npos = 6, .px = "excavate", .lr = "m", .fj = 12, .ja = "+", .i = "b", .o = "o"},
    {.npos = 6, .px = "excavate", .lr = "ig", .fj = 12, .ja = "-", .i = "b", .o = "o"},
    {.header = "gene rewriting"},
    {.npos = 6, .px = "uplift", .lr = "f", .fj = 20, .ja = "-", .i = "l", .o = "p"},
    {.header = "replication"},
    {.npos = 3, .px = "mechanize", .lr = "mrsoijx", .fj = 20, .has_ii = 1, .ii = 38, .go = "f"},
    {.header = "biome hacking"},
    {.npos = 3, .px = "terraform", .lr = "xisoj", .fj = 20, .has_ii = 1, .ii = 39, .go = "e"},
    {.header = "starbirth"},
    {.npos = 3, .px = "collapse", .lr = "gm", .fj = 30, .has_ii = 1, .ii = 41, .cw = "protostar", .has_rn = 1, .rn = 45},
    {.header = "geoharvesting"},
    {.npos = 3, .px = "harvest", .lr = "efmorxjisg", .fj = -20, .has_ii = 1, .ii = 23, .cw = "burst", .has_rn = 1,
        .rn = 46},
};

static const pico8_jb_item_t dr_1_1_items[] = {{.s = "-lab-"}};
static const pico8_jb_row_t dr_1_1 = {.count = 1, .items = dr_1_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_1_2_items[] = {{.s = "makes"}, {.n = 24}, {.s = "science when supplied"}};
static const pico8_jb_row_t dr_1_2 = {.count = 3, .items = dr_1_2_items};
static const pico8_jb_item_t dr_1_3_items[] = {{.s = "with"}, {.n = 19}, {.s = "and any resource"}};
static const pico8_jb_row_t dr_1_3 = {.count = 3, .items = dr_1_3_items};
static const pico8_jb_item_t dr_1_4_items[] = {{.s = "adding more of that resource"}};
static const pico8_jb_row_t dr_1_4 = {.count = 1, .items = dr_1_4_items};
static const pico8_jb_item_t dr_1_5_items[] = {{.s = "increases output"}};
static const pico8_jb_row_t dr_1_5 = {.count = 1, .items = dr_1_5_items};
static const pico8_jb_item_t dr_1_items[] = {{.row = &dr_1_1}, {.row = &dr_1_2}, {.row = &dr_1_3}, {.row = &dr_1_4},
    {.row = &dr_1_5}};
static const pico8_jb_item_t dr_2_1_items[] = {{.s = "-food processor-"}};
static const pico8_jb_row_t dr_2_1 = {.count = 1, .items = dr_2_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_2_2_items[] = {{.s = "converts"}, {.n = 20}, {.s = "into"}, {.n = 28}};
static const pico8_jb_row_t dr_2_2 = {.count = 4, .items = dr_2_2_items};
static const pico8_jb_item_t dr_2_items[] = {{.row = &dr_2_1}, {.row = &dr_2_2}};
static const pico8_jb_item_t dr_3_1_items[] = {{.s = "-slipgate-"}};
static const pico8_jb_row_t dr_3_1 = {.count = 1, .items = dr_3_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_3_2_items[] = {{.s = "connects up to 3 slipways"}};
static const pico8_jb_row_t dr_3_2 = {.count = 1, .items = dr_3_2_items};
static const pico8_jb_item_t dr_3_items[] = {{.row = &dr_3_1}, {.row = &dr_3_2}};
static const pico8_jb_item_t dr_4_1_items[] = {{.s = "-infragate-"}};
static const pico8_jb_row_t dr_4_1 = {.count = 1, .items = dr_4_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_4_2_items[] = {{.s = "connects up to 3 slipways"}};
static const pico8_jb_row_t dr_4_2 = {.count = 1, .items = dr_4_2_items};
static const pico8_jb_item_t dr_4_3_items[] = {{.s = "routes through infragates"}};
static const pico8_jb_row_t dr_4_3 = {.count = 1, .items = dr_4_3_items};
static const pico8_jb_item_t dr_4_4_items[] = {{.s = "can cross other slipways"}};
static const pico8_jb_row_t dr_4_4 = {.count = 1, .items = dr_4_4_items};
static const pico8_jb_item_t dr_4_items[] = {{.row = &dr_4_1}, {.row = &dr_4_2}, {.row = &dr_4_3}, {.row = &dr_4_4}};
static const pico8_jb_item_t dr_5_1_items[] = {{.s = "-trading hub-"}};
static const pico8_jb_row_t dr_5_1 = {.count = 1, .items = dr_5_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_5_2_items[] = {{.s = "generates"}, {.n = 25}, {.n = 23}, {.s = "once you connect"}};
static const pico8_jb_row_t dr_5_2 = {.count = 4, .items = dr_5_2_items};
static const pico8_jb_item_t dr_5_3_items[] = {{.s = "2 or 3 different resources"}};
static const pico8_jb_row_t dr_5_3 = {.count = 1, .items = dr_5_3_items};
static const pico8_jb_item_t dr_5_items[] = {{.row = &dr_5_1}, {.row = &dr_5_2}, {.row = &dr_5_3}};
static const pico8_jb_item_t dr_6_1_items[] = {{.s = "-synthesizer-"}};
static const pico8_jb_row_t dr_6_1 = {.count = 1, .items = dr_6_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_6_2_items[] = {{.s = "can provide"}, {.n = 28}, {.n = 20}, {.n = 21}, {.n = 22}};
static const pico8_jb_row_t dr_6_2 = {.count = 5, .items = dr_6_2_items};
static const pico8_jb_item_t dr_6_3_items[] = {{.s = "to a planet that needs it"}};
static const pico8_jb_row_t dr_6_3 = {.count = 1, .items = dr_6_3_items};
static const pico8_jb_item_t dr_6_items[] = {{.row = &dr_6_1}, {.row = &dr_6_2}, {.row = &dr_6_3}};
static const pico8_jb_item_t dr_7_1_items[] = {{.s = "-ascension gate-"}};
static const pico8_jb_row_t dr_7_1 = {.count = 1, .items = dr_7_1_items, .has_c = 1, .c = 9};
static const pico8_jb_item_t dr_7_2_items[] = {{.s = "accepts"}, {.n = 19}, {.s = "and turns them"}};
static const pico8_jb_row_t dr_7_2 = {.count = 3, .items = dr_7_2_items};
static const pico8_jb_item_t dr_7_3_items[] = {{.s = "into"}, {.n = 25}, {.s = "happiness"}};
static const pico8_jb_row_t dr_7_3 = {.count = 3, .items = dr_7_3_items};
static const pico8_jb_item_t dr_7_items[] = {{.row = &dr_7_1}, {.row = &dr_7_2}, {.row = &dr_7_3}};

const pico8_jb_row_t pico8_dr_table[PICO8_DR_COUNT] = {
    {.count = 5, .items = dr_1_items},
    {.count = 2, .items = dr_2_items},
    {.count = 2, .items = dr_3_items},
    {.count = 4, .items = dr_4_items},
    {.count = 3, .items = dr_5_items},
    {.count = 3, .items = dr_6_items},
    {.count = 3, .items = dr_7_items},
};

static const pico8_jb_item_t nb_1_oy_items[] = {{.s = "longer range"}, {.s = "on slipways and probes"}};
static const pico8_jb_row_t nb_1_oy = {.count = 2, .items = nb_1_oy_items};
static const pico8_jb_item_t nb_2_oy_1_items[] = {{.s = "use"}, {.n = 102}, {.n = 105}, {.s = "planets"}};
static const pico8_jb_row_t nb_2_oy_1 = {.count = 4, .items = nb_2_oy_1_items};
static const pico8_jb_item_t nb_2_oy_2_items[] = {{.s = "to settle"}, {.n = 19}, {.s = "and farm"}, {.n = 28}};
static const pico8_jb_row_t nb_2_oy_2 = {.count = 4, .items = nb_2_oy_2_items};
static const pico8_jb_item_t nb_2_oy_items[] = {{.row = &nb_2_oy_1}, {.row = &nb_2_oy_2}};
static const pico8_jb_row_t nb_2_oy = {.count = 2, .items = nb_2_oy_items};
static const pico8_jb_item_t nb_3_oy_1_items[] = {{.s = "colonizing planets"}};
static const pico8_jb_row_t nb_3_oy_1 = {.count = 1, .items = nb_3_oy_1_items};
static const pico8_jb_item_t nb_3_oy_2_items[] = {{.s = "becomes 15% cheaper"}};
static const pico8_jb_row_t nb_3_oy_2 = {.count = 1, .items = nb_3_oy_2_items};
static const pico8_jb_item_t nb_3_oy_items[] = {{.row = &nb_3_oy_1}, {.row = &nb_3_oy_2}};
static const pico8_jb_row_t nb_3_oy = {.count = 2, .items = nb_3_oy_items};
static const pico8_jb_item_t nb_4_oy_1_items[] = {{.s = "build mines using"}, {.n = 18}};
static const pico8_jb_row_t nb_4_oy_1 = {.count = 2, .items = nb_4_oy_1_items};
static const pico8_jb_item_t nb_4_oy_2_items[] = {{.s = "on"}, {.n = 106}, {.n = 105}, {.n = 110}, {.s = "planets"}};
static const pico8_jb_row_t nb_4_oy_2 = {.count = 5, .items = nb_4_oy_2_items};
static const pico8_jb_item_t nb_4_oy_items[] = {{.row = &nb_4_oy_1}, {.row = &nb_4_oy_2}};
static const pico8_jb_row_t nb_4_oy = {.count = 2, .items = nb_4_oy_items};
static const pico8_jb_item_t nb_5_oy_1_items[] = {{.s = "allow farming"}, {.n = 28}, {.s = "on"}, {.n = 107},
    {.n = 108}};
static const pico8_jb_row_t nb_5_oy_1 = {.count = 5, .items = nb_5_oy_1_items};
static const pico8_jb_item_t nb_5_oy_2_items[] = {{.s = "improve"}, {.n = 28}, {.s = "output of"}, {.n = 103},
    {.n = 104}, {.n = 119}};
static const pico8_jb_row_t nb_5_oy_2 = {.count = 6, .items = nb_5_oy_2_items};
static const pico8_jb_item_t nb_5_oy_items[] = {{.row = &nb_5_oy_1}, {.row = &nb_5_oy_2}};
static const pico8_jb_row_t nb_5_oy = {.count = 2, .items = nb_5_oy_items};
static const pico8_jb_item_t nb_6_oy_1_items[] = {{.s = "build"}, {.n = 109}, {.s = "slipgates"}};
static const pico8_jb_row_t nb_6_oy_1 = {.count = 3, .items = nb_6_oy_1_items};
static const pico8_jb_item_t nb_6_oy_2_items[] = {{.s = "to extend your slipways"}};
static const pico8_jb_row_t nb_6_oy_2 = {.count = 1, .items = nb_6_oy_2_items};
static const pico8_jb_item_t nb_6_oy_items[] = {{.row = &nb_6_oy_1}, {.row = &nb_6_oy_2}};
static const pico8_jb_row_t nb_6_oy = {.count = 2, .items = nb_6_oy_items};
static const pico8_jb_item_t nb_7_oy_1_items[] = {{.s = "destroy empty planets"}};
static const pico8_jb_row_t nb_7_oy_1 = {.count = 1, .items = nb_7_oy_1_items};
static const pico8_jb_item_t nb_7_oy_2_items[] = {{.s = "for an instant 20"}, {.n = 23}, {.s = "boost"}};
static const pico8_jb_row_t nb_7_oy_2 = {.count = 3, .items = nb_7_oy_2_items};
static const pico8_jb_item_t nb_7_oy_items[] = {{.row = &nb_7_oy_1}, {.row = &nb_7_oy_2}};
static const pico8_jb_row_t nb_7_oy = {.count = 2, .items = nb_7_oy_items};
static const pico8_jb_item_t nb_8_oy_1_items[] = {{.s = "build"}, {.n = 120}, {.s = "trading hubs to"}};
static const pico8_jb_row_t nb_8_oy_1 = {.count = 3, .items = nb_8_oy_1_items};
static const pico8_jb_item_t nb_8_oy_2_items[] = {{.s = "turn 2-3 different resources"}};
static const pico8_jb_row_t nb_8_oy_2 = {.count = 1, .items = nb_8_oy_2_items};
static const pico8_jb_item_t nb_8_oy_3_items[] = {{.s = "into"}, {.n = 25}, {.s = "happiness and"}, {.n = 23},
    {.s = "money"}};
static const pico8_jb_row_t nb_8_oy_3 = {.count = 5, .items = nb_8_oy_3_items};
static const pico8_jb_item_t nb_8_oy_items[] = {{.row = &nb_8_oy_1}, {.row = &nb_8_oy_2}, {.row = &nb_8_oy_3}};
static const pico8_jb_row_t nb_8_oy = {.count = 3, .items = nb_8_oy_items};
static const pico8_jb_item_t nb_9_oy_1_items[] = {{.s = "turn any of"}, {.n = 102}, {.n = 104}, {.n = 105}, {.n = 106},
    {.n = 107}, {.n = 108}};
static const pico8_jb_row_t nb_9_oy_1 = {.count = 7, .items = nb_9_oy_1_items};
static const pico8_jb_item_t nb_9_oy_2_items[] = {{.s = "into"}, {.n = 118}, {.s = "forgeworlds"}};
static const pico8_jb_row_t nb_9_oy_2 = {.count = 3, .items = nb_9_oy_2_items};
static const pico8_jb_item_t nb_9_oy_items[] = {{.row = &nb_9_oy_1}, {.row = &nb_9_oy_2}};
static const pico8_jb_row_t nb_9_oy = {.count = 2, .items = nb_9_oy_items};
static const pico8_jb_item_t nb_10_oy_1_items[] = {{.s = "colonizing planets"}};
static const pico8_jb_row_t nb_10_oy_1 = {.count = 1, .items = nb_10_oy_1_items};
static const pico8_jb_item_t nb_10_oy_2_items[] = {{.s = "takes 2 months instead of 3"}};
static const pico8_jb_row_t nb_10_oy_2 = {.count = 1, .items = nb_10_oy_2_items};
static const pico8_jb_item_t nb_10_oy_items[] = {{.row = &nb_10_oy_1}, {.row = &nb_10_oy_2}};
static const pico8_jb_row_t nb_10_oy = {.count = 2, .items = nb_10_oy_items};
static const pico8_jb_item_t nb_11_oy_1_items[] = {{.s = "earn 15% more"}, {.n = 23}, {.s = "income"}};
static const pico8_jb_row_t nb_11_oy_1 = {.count = 3, .items = nb_11_oy_1_items};
static const pico8_jb_item_t nb_11_oy_items[] = {{.row = &nb_11_oy_1}, {.s = "from trade"}};
static const pico8_jb_row_t nb_11_oy = {.count = 2, .items = nb_11_oy_items};
static const pico8_jb_item_t nb_12_oy_1_items[] = {{.s = "collapse"}, {.n = 110}, {.n = 106}, {.s = "into stars"}};
static const pico8_jb_row_t nb_12_oy_1 = {.count = 4, .items = nb_12_oy_1_items};
static const pico8_jb_item_t nb_12_oy_2_items[] = {{.s = "to make"}, {.n = 54}, {.s = "that fills any need"}};
static const pico8_jb_row_t nb_12_oy_2 = {.count = 3, .items = nb_12_oy_2_items};
static const pico8_jb_item_t nb_12_oy_3_items[] = {{.s = "and increases production"}};
static const pico8_jb_row_t nb_12_oy_3 = {.count = 1, .items = nb_12_oy_3_items};
static const pico8_jb_item_t nb_12_oy_items[] = {{.row = &nb_12_oy_1}, {.row = &nb_12_oy_2}, {.row = &nb_12_oy_3}};
static const pico8_jb_row_t nb_12_oy = {.count = 3, .items = nb_12_oy_items};
static const pico8_jb_item_t nb_13_oy_1_items[] = {{.s = "terraform"}, {.n = 102}, {.n = 104}, {.n = 105}, {.n = 107},
    {.n = 108}};
static const pico8_jb_row_t nb_13_oy_1 = {.count = 6, .items = nb_13_oy_1_items};
static const pico8_jb_item_t nb_13_oy_2_items[] = {{.s = "into"}, {.n = 103}, {.s = "earth-like worlds"}};
static const pico8_jb_row_t nb_13_oy_2 = {.count = 3, .items = nb_13_oy_2_items};
static const pico8_jb_item_t nb_13_oy_items[] = {{.row = &nb_13_oy_1}, {.row = &nb_13_oy_2}};
static const pico8_jb_row_t nb_13_oy = {.count = 2, .items = nb_13_oy_items};
static const pico8_jb_item_t nb_14_oy_1_items[] = {{.s = "build"}, {.n = 121}, {.s = "infragates"}};
static const pico8_jb_row_t nb_14_oy_1 = {.count = 3, .items = nb_14_oy_1_items};
static const pico8_jb_item_t nb_14_oy_2_items[] = {{.s = "that let slipways pass"}};
static const pico8_jb_row_t nb_14_oy_2 = {.count = 1, .items = nb_14_oy_2_items};
static const pico8_jb_item_t nb_14_oy_3_items[] = {{.s = "under other slipways"}};
static const pico8_jb_row_t nb_14_oy_3 = {.count = 1, .items = nb_14_oy_3_items};
static const pico8_jb_item_t nb_14_oy_items[] = {{.row = &nb_14_oy_1}, {.row = &nb_14_oy_2}, {.row = &nb_14_oy_3}};
static const pico8_jb_row_t nb_14_oy = {.count = 3, .items = nb_14_oy_items};
static const pico8_jb_item_t nb_15_oy_2_items[] = {{.s = "2/5/8"}, {.n = 24}, {.s = "based on level"}};
static const pico8_jb_row_t nb_15_oy_2 = {.count = 3, .items = nb_15_oy_2_items};
static const pico8_jb_item_t nb_15_oy_items[] = {{.s = "labs now generate"}, {.row = &nb_15_oy_2}};
static const pico8_jb_row_t nb_15_oy = {.count = 2, .items = nb_15_oy_items};
static const pico8_jb_item_t nb_16_oy_1_items[] = {{.s = "build synthesizers"}};
static const pico8_jb_row_t nb_16_oy_1 = {.count = 1, .items = nb_16_oy_1_items};
static const pico8_jb_item_t nb_16_oy_2_items[] = {{.s = "that create"}, {.n = 28}, {.n = 20}, {.n = 21}, {.n = 22}};
static const pico8_jb_row_t nb_16_oy_2 = {.count = 5, .items = nb_16_oy_2_items};
static const pico8_jb_item_t nb_16_oy_items[] = {{.row = &nb_16_oy_1}, {.row = &nb_16_oy_2}};
static const pico8_jb_row_t nb_16_oy = {.count = 2, .items = nb_16_oy_items};
static const pico8_jb_item_t nb_17_oy_1_items[] = {{.s = "colonies yield more"}, {.n = 19}};
static const pico8_jb_row_t nb_17_oy_1 = {.count = 2, .items = nb_17_oy_1_items};
static const pico8_jb_item_t nb_17_oy_2_items[] = {{.s = "you can uplift"}, {.n = 20}, {.n = 13}, {.n = 19}};
static const pico8_jb_row_t nb_17_oy_2 = {.count = 4, .items = nb_17_oy_2_items};
static const pico8_jb_item_t nb_17_oy_3_items[] = {{.s = "using"}, {.n = 118}, {.s = "forgeworlds"}};
static const pico8_jb_row_t nb_17_oy_3 = {.count = 3, .items = nb_17_oy_3_items};
static const pico8_jb_item_t nb_17_oy_items[] = {{.row = &nb_17_oy_1}, {.row = &nb_17_oy_2}, {.row = &nb_17_oy_3}};
static const pico8_jb_row_t nb_17_oy = {.count = 3, .items = nb_17_oy_items};
static const pico8_jb_item_t nb_18_oy_1_items[] = {{.s = "adds 3 months to each"}, {.n = 15}, {.s = "year"}};
static const pico8_jb_row_t nb_18_oy_1 = {.count = 3, .items = nb_18_oy_1_items};
static const pico8_jb_item_t nb_18_oy_items[] = {{.row = &nb_18_oy_1}};
static const pico8_jb_row_t nb_18_oy = {.count = 1, .items = nb_18_oy_items};
static const pico8_jb_item_t nb_19_oy_1_items[] = {{.s = "build"}, {.n = 127}, {.s = "ascension gates"}};
static const pico8_jb_row_t nb_19_oy_1 = {.count = 3, .items = nb_19_oy_1_items};
static const pico8_jb_item_t nb_19_oy_2_items[] = {{.s = "to turn"}, {.n = 19}, {.s = "into"}, {.n = 25},
    {.s = "happiness"}};
static const pico8_jb_row_t nb_19_oy_2 = {.count = 5, .items = nb_19_oy_2_items};
static const pico8_jb_item_t nb_19_oy_items[] = {{.row = &nb_19_oy_1}, {.row = &nb_19_oy_2}};
static const pico8_jb_row_t nb_19_oy = {.count = 2, .items = nb_19_oy_items};

const pico8_nb_entry_t pico8_nb_table[PICO8_NB_COUNT] = {
    {"space folding", 1, "t", 22, 4, &nb_1_oy},
    {"geneseeds", 1, "l", 20, 5, &nb_2_oy},
    {"nanomaterials", 1, "o", 21, 6, &nb_3_oy},
    {"drillbots", 1, "b", 18, 7, &nb_4_oy},
    {"xenofoods", 2, "l", 20, 10, &nb_5_oy},
    {"slipgates", 2, "t", 22, 12, &nb_6_oy},
    {"geoharvesting", 2, "o", 21, 14, &nb_7_oy},
    {"trade league", 2, "t", 22, 16, &nb_8_oy},
    {"replication", 2, "b", 18, 18, &nb_9_oy},
    {"autoassemblers", 3, "b", 18, 19, &nb_10_oy},
    {"skill implants", 3, "b", 18, 22, &nb_11_oy},
    {"starbirth", 3, "o", 21, 25, &nb_12_oy},
    {"biome hacking", 3, "l", 20, 28, &nb_13_oy},
    {"infraspace", 3, "t", 22, 31, &nb_14_oy},
    {"superhuman ai", 4, "t", 22, 34, &nb_15_oy},
    {"void synthesis", 4, "o", 21, 38, &nb_16_oy},
    {"gene rewriting", 4, "l", 20, 42, &nb_17_oy},
    {"time compression", 4, "b", 18, 46, &nb_18_oy},
    {"ascension", 4, "a", 33, 50, &nb_19_oy},
};

int pico8_r_lookup(const char *key, int *out_qe) {
    int i;

    assert(key != NULL && "r lookup: key must be a string -- a number never keys r");
    for (i = 0; i < PICO8_R_COUNT; i++) {
        if (strcmp(pico8_r_table[i].key, key) == 0) {
            *out_qe = pico8_r_table[i].qe;
            return 1;
        }
    }
    return 0;
}

const char *pico8_iz_lookup(const char *key) {
    int i;

    assert(key != NULL && "iz lookup: key must be a string -- a number never keys iz");
    for (i = 0; i < PICO8_IZ_COUNT; i++) {
        if (strcmp(pico8_iz_table[i].key, key) == 0) {
            return pico8_iz_table[i].name;
        }
    }
    return NULL;
}

const pico8_hc_entry_t *pico8_hc_lookup(const char *key) {
    int i;

    assert(key != NULL && "hc lookup: key must be a string -- a number never keys hc");
    for (i = 0; i < PICO8_HC_COUNT; i++) {
        if (strcmp(pico8_hc_table[i].key, key) == 0) {
            return &pico8_hc_table[i];
        }
    }
    return NULL;
}

const pico8_nb_entry_t *pico8_nb_lookup(const char *px) {
    int i;

    for (i = 0; px != NULL && i < PICO8_NB_COUNT; i++) {
        if (strcmp(pico8_nb_table[i].px, px) == 0) {
            return &pico8_nb_table[i];
        }
    }
    return NULL;
}
