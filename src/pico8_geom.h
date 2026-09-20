#ifndef PICO8_GEOM_H
#define PICO8_GEOM_H

#include "pico8_fixed.h"
#include "pico8_vec.h"

/* Pure geometry helpers (pico8/lua.lua lines 1475-1494): segment-segment
 * crossing (cl), signed-side-of-a-line (ee), and point-near-segment (be).
 * No entity/global-state dependency -- these operate purely on pico8_vec_t
 * points and pico8_fixed_t numbers, matching the PRD's "Geometry module"
 * Implementation Decision.
 *
 * Tri-state design for `ee`: the original Lua's `ee(a,b,c)` returns `nil`
 * (falsy) when the cross product `kl` is exactly zero ("c is on the line
 * a->b"), or `sgn(kl)` (always +1 or -1, since `sgn` is only ever called
 * here once `kl~=0` is already known) otherwise. `cl` needs to tell "was
 * on the line" apart from "was not on the line, and here is which side" --
 * its own `oq1 and oq2 and op1 and op2 and oq1~=oq2 and op1~=op2` check
 * first requires all four results to be non-nil (not on the line), *then*
 * compares two of them for a differing sign. Coercing `ee`'s result to a
 * plain C `bool`, or returning bare `pico8_fixed_t` sign values with 0
 * doing double duty for "on the line", would each lose one half of that
 * distinction. This port instead uses a dedicated three-value enum: one
 * value for "on the line", and two others for "positive"/"negative" cross
 * product -- a real C type distinct from a boolean or from pico8_fixed_t,
 * so a caller cannot silently misuse it as either. */
typedef enum {
    PICO8_EE_ON_LINE = 0,
    PICO8_EE_POSITIVE = 1,
    PICO8_EE_NEGATIVE = -1
} pico8_ee_side_t;

/* function ee(a,b,c)
 *   local kl=
 *   (b.y-a.y)*(c.x-b.x)-
 *   (b.x-a.x)*(c.y-b.y)
 *   return kl~=0 and sgn(kl)
 * end
 * Signed side of point c relative to line a->b: PICO8_EE_ON_LINE when the
 * cross product (b-a) x (c-a) is exactly zero, else PICO8_EE_POSITIVE or
 * PICO8_EE_NEGATIVE matching sgn(kl)'s sign (kl is never 0 in that branch,
 * so sgn(kl) here is a genuine mathematical sign, not sgn's own
 * documented sgn(0)==1 special case). */
pico8_ee_side_t pico8_geom_ee(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c);

/* function cl(p1,p2,q1,q2)
 *   local oq1,oq2,op1,op2=
 *   ee(p1,p2,q1),ee(p1,p2,q2),
 *   ee(q1,q2,p1),ee(q1,q2,p2)
 *   return oq1 and oq2 and op1 and op2
 *   and oq1~=oq2 and op1~=op2
 * end
 * Segment-segment crossing test: true iff p1/p2 are on opposite sides of
 * segment q1-q2 *and* q1/q2 are on opposite sides of segment p1-p2 (and
 * none of the four points lies exactly on the other segment's line).
 * Returns 1/0, matching this codebase's existing int-as-bool convention
 * (see pico8_input.h's pico8_btn/pico8_stat). */
int pico8_geom_cl(pico8_vec_t p1, pico8_vec_t p2, pico8_vec_t q1, pico8_vec_t q2);

/* function be(a,b,c,fh)
 *   local d=b-a
 *   local l_=d:rg(c-a)/#d
 *   return l_>0.01
 *   and l_<0.99
 *   and #(a+d*l_-c)<fh
 * end
 * Point-near-segment test: projects c onto the line a-b as a fraction l_
 * of d=b-a (via dot(d,c-a)/lenSq(d), i.e. #d is the *squared* length --
 * PICO-8's __len on a km vector, see pico8_vec_len_sq), requires l_ to
 * land strictly within (0.01, 0.99) -- excluding a small margin at each
 * endpoint, copied verbatim -- and then requires the *squared* distance
 * from c to the projected point (again via #, squared length, not qx's
 * actual length) to be less than fh. fh is therefore itself a
 * squared-distance threshold in this port, exactly as in the original.
 * Returns 1/0, matching this codebase's existing int-as-bool convention. */
int pico8_geom_be(pico8_vec_t a, pico8_vec_t b, pico8_vec_t c, pico8_fixed_t fh);

#endif
