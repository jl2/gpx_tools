// gpxpoint.h

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

#ifndef GPX_POINT_H
#define GPX_POINT_H

#include <QString>
#include <QDateTime>

#include "gpxelement.h"

class GpxPoint : public GpxElement {
public:
    // Default constructor
    GpxPoint(double latitude=0.0, double longitude=0.0, double elev=0.0, QDateTime timev=QDateTime());
    void setLatLon(double latitude, double longitude);

    // Compute the distance between two GPX points
    double distanceTo(const GpxPoint &p2);
    double speedBetween(const GpxPoint &p2);
    time_t secondsBetween(const GpxPoint &p2);

    double latitude();
    double longitude();

    double elevation();

    QDateTime time();

    double x();
    double y();

    bool north();
    int zone();

    void toXml(QString &xmlStr);

private:
    // Latitude, longitude and elevation straight from the GPX file
    double _lat, _lon;
    double _ele;

    // UTM coordinates
    double _x, _y;
    bool _north;
    int _zone;

    // Time from the GPX file
    QDateTime _time;
};

#endif
