#ifndef MARPAWRAPPER_AMALGAMATION
#define MARPAWRAPPER_AMALGAMATION

#include "../libmarpa/work/stage/marpa_ami.c"
#include "../libmarpa/work/stage/marpa_avl.c"
#include "../libmarpa/work/stage/marpa.c"
/* marpa_codes includes marpa_codes.h that is not protected - c.f. the usage of MARPAWRAPPER_AMALGAMATION in internal/_logging.h */
#include "../libmarpa/work/stage/marpa_codes.c"
#include "../libmarpa/work/stage/marpa_obs.c"
#include "../libmarpa/work/stage/marpa_tavl.c"
#include "../src/asf.c"
#include "../src/grammar.c"
#include "../src/recognizer.c"
#include "../src/value.c"

#endif /* MARPAWRAPPER_AMALGAMATION */
