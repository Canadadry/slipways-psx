# 1a.20 progress
- start: read PRD, c868719
- chunk 1: had_qw guard in test_pico8_qh fold_descendant on OLD code -> test_pico8_qh all passed, frozen values unchanged
- chunk 1: test_pico8_os characterization frozen on OLD code: BUILT 0x0FBFD36B, LIFE 0x56980D6D (db processor 131136 = half of bare 262272), LONG 0x47B58E4E, 27273 frames, 1 processor. Saved scratchpad/test_pico8_os.old-code.c
- chunk 2: os converted; os/qh/jo/nl all pass, frozen values unchanged
- mutations on test_pico8_os: 20/21 caught; od not applied survived (equivalent: 15 == qh's od)
- coverage mutation (record removed): class-coverage-net exit 2, os 0 0/15 -> caught; restored
