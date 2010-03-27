#include "track.h"
#include <ctime>

double Track::averageSpeed() {
    if (duration()>0) {
        return length()/duration();
    }
    return 0.0;
}
