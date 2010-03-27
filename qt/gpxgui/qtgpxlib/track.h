// track.h

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

#ifndef GPX_TRACK_H
#define GPX_TRACK_H

#include <ctime>

class Track {
public:
    virtual double length()=0;
    virtual time_t duration()=0;
    virtual double maxSpeed() = 0;
    virtual double averageSpeed() {
        if (duration()>0) {
            return length()/duration();
        }
        return 0.0;
    }
    virtual void boundLatLon(double &minLat, double &minLon, double &minEle,
                     double &maxLat, double &maxLon, double &maxEle) = 0;

    virtual void boundUTM(double &minX, double &minY, double &minEle,
                  double &maxX, double &maxY, double &maxEle) = 0;

protected:
    Track() { }
    Track& operator =(const Track&) { return *this;}
};

#endif
