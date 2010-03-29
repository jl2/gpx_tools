// unitconversion.cpp

// Copyright (c) 2010, Jeremiah LaRocco jeremiah.larocco@gmail.com

// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.

// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "unitconversion.h"

double meter2mile(double len) {
    return len * 0.000621371192;
}

double meterPerSecond2MilePerHour(double speed) {
    return speed * 2.23693629;
}

QString formatDuration(time_t dur, bool showBlanks) {
    int secs = dur % 60;
    int mins = (dur/60)%60;
    int hrs = dur/(60*60);
    QChar z('0');
    QString hhmmss = "%1:%2:%3";
    QString mmss = "%1:%2";
    QString ss = "%1";
    
    if (showBlanks) {
        mmss = "00:" + mmss;
        ss = "00:00:" + ss;
    }

    if (hrs>0) {
        return QString(hhmmss).arg(hrs,2,10,z).arg(mins,2,10,z).arg(secs,2,10,z);
    } else if (mins>0) {
        return QString(mmss).arg(mins,2,10,z).arg(secs,2,10,z);
    }
    return QString(ss).arg(secs,2L,10,z);
}
