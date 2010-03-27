// gpxtracksegment.h

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

#ifndef GPX_TRACK_SEGMENT_H
#define GPX_TRACK_SEGMENT_H

#include "gpxtracksegment.h"

#include "gpxpoint.h"

#include "gpxelement.h"
#include "track.h"

#include <QString>
#include <QList>

class GpxTrackSegment : public GpxElement, public Track {
public:
    GpxTrackSegment();

    GpxPoint& operator [](int n);
    void addPoint(const GpxPoint &pt);

    GpxPoint &lastPoint();

    QString name();
    void setName(const QString &name);
    
    int number();
    void setNumber(int number);

    int pointCount();

    double length();
    time_t duration();
    double maxSpeed();

    void toXml(QString &xmlStr);

private:
    QString _name;

    // number and track_pts are optional
    int _number;
    QList<GpxPoint> track_pts;
};

#endif
