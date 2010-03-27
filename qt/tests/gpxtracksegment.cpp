// gpxtracksegment.cpp

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

#include "gpxtracksegment.h"

#include <cassert>

GpxTrackSegment::GpxTrackSegment() : _name(""), _number(0) { }

// Convert to an XML string;
void GpxTrackSegment::toXml(QString &xmlStr) {
    xmlStr += QString("<trk><name>%1</name>").arg(_name);
    if (_number > 0) {
        xmlStr += QString("<number>%1</number>").arg(_number);
    }

    xmlStr += "<trkseg>";
    for (int i=0; i<track_pts.size(); ++i) {
        track_pts[i].toXml(xmlStr);
    }
    xmlStr += "</trkseg></trk>";
}

// Calculate the length of the track segment
double GpxTrackSegment::length() {
    double dist = 0.0;
    for (int i=0; i< track_pts.size()-1; ++i) {
        dist += track_pts[i].distanceTo(track_pts[i+1]);
    }
    return dist;
}

time_t GpxTrackSegment::duration() {
    if (track_pts.size()<2) return 0;
    return track_pts[0].secondsBetween(track_pts[track_pts.size()-1]);
}

double GpxTrackSegment::maxSpeed() {
    double curMax = 0.0;
    for (int i=0; i< track_pts.size()-1; ++i) {

        double spd = track_pts[i].speedBetween(track_pts[i+1]);

        if (spd > curMax) {
            curMax = spd;
        }
    }
    return curMax;
}

GpxPoint& GpxTrackSegment::operator [](int n) {
    assert(n<track_pts.size());

    return track_pts[n];
}

void GpxTrackSegment::addPoint(const GpxPoint &pt) {
    track_pts.push_back(pt);
}

GpxPoint &GpxTrackSegment::lastPoint() {
    assert(track_pts.size()>0);
    return track_pts[track_pts.size()-1];
}
QString GpxTrackSegment::name() {
    return _name;
}
void GpxTrackSegment::setName(const QString &name) {
    _name = name;
}
    
int GpxTrackSegment::number() {
    return _number;
}
void GpxTrackSegment::setNumber(int number) {
    _number = number;
}

int GpxTrackSegment::pointCount() {
    return track_pts.size();
}
