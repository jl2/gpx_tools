// gpxpoint.cpp

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

#include "gpxpoint.h"

#include <GeographicLib/UTMUPS.hpp>

// Default constructor
GpxPoint::GpxPoint(double latitude, double longitude, double elev, QDateTime timev) : _lat(latitude), _lon(longitude), _ele(elev), _time(timev) {
    setLatLon(latitude, longitude);
}
void GpxPoint::setLatLon(double latitude, double longitude) {
    _lat = latitude;
    _lon = longitude;

    double gamma;
    double k;
    GeographicLib::UTMUPS::Forward(_lat, _lon, _zone, _north, _x, _y, gamma, k);
}

// Convert to an XML string
void GpxPoint::toXml(QString &xmlStr) {
    xmlStr += QString("<trkpt lat=\"%1\" lon=\"%2\">"
                      "<ele>%3</ele><time>%4</time>"
                      "</trkpt>")
        .arg(_lat, 0, 'f', 9)
        .arg(_lon, 0, 'f', 9)
        .arg(_ele, 0, 'f', 9)
        .arg(_time.toString(Qt::ISODate));
}
// Compute the distance between two GPX points
double GpxPoint::distanceTo(const GpxPoint &p2) {
    double dx = _x - p2._x;
    double dy = _y - p2._y;
    double dz = _ele - p2._ele;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

double GpxPoint::speedBetween(const GpxPoint &p2) {
    double dist = distanceTo(p2);
    double dt = p2._time.toTime_t() - _time.toTime_t();
    return dist/dt;
}

time_t GpxPoint::secondsBetween(const GpxPoint &p2) {
    return p2._time.toTime_t() - _time.toTime_t();
}
double GpxPoint::latitude() {
    return _lat;
}
double GpxPoint::longitude() {
    return _lon;
}

double GpxPoint::elevation() {
    return _ele;
}

QDateTime GpxPoint::time() {
    return _time;
}

double GpxPoint::x() {
    return _x;
}
double GpxPoint::y() {
    return _y;
}

bool GpxPoint::north() {
    return _north;
}
int GpxPoint::zone() {
    return _zone;
}
