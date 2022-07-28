#ifndef MARPAWRAPPER_AMALGAMATION
#define MARPAWRAPPER_AMALGAMATION

#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa_ami.c"
#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa_avl.c"
#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa.c"
/* marpa_codes includes marpa_codes.h that is not protected - c.f. the usage of MARPAWRAPPER_AMALGAMATION in internal/_logging.h */
#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa_codes.c"
#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa_obs.c"
#include "../3rdparty/github/Marpa--R2/cpan/engine/read_only/marpa_tavl.c"
#include "../src/asf.c"
#include "../src/grammar.c"
#include "../src/recognizer.c"
#include "../src/value.c"

#endif /* MARPAWRAPPER_AMALGAMATION */
