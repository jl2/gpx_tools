// gpxfile.cpp

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

#include "gpxfile.h"

#include <cassert>

GpxFile::GpxFile(QString fname, bool purgeEmpty) : _time(QDateTime()) {
    readFile(fname, purgeEmpty);
}
    
void GpxFile::toXml(QString &xmlStr) {
    xmlStr += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<gpx version=\"1.0\" creator=\"Whatever\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xmlns=\"http://www.topografix.com/GPX/1/0\" "
        "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 "
        "http://www.topografix.com/GPX/1/0/gpx.xsd\">";
    if (_time.isValid()) {
        xmlStr += "<time>" + _time.toString(Qt::ISODate) + "</time>";
    }
    for (int i=0; i<track_segments.size(); ++i) {
        track_segments[i].toXml(xmlStr);
    }
    xmlStr += "</gpx>\n";
}

double GpxFile::length() {
    double dist = 0.0;
    for (int i=0; i< track_segments.size(); ++i) {
        dist += track_segments[i].length();
    }
    return dist;
}

double GpxFile::maxSpeed() {
    if (track_segments.size()==0) return 0.0;
    double maxs = 0.0;
        
    for (int i=0; i<track_segments.size(); ++i) {
        double spd = track_segments[i].maxSpeed();
        if (spd > maxs) {
            maxs = spd;
        }
    }
    return maxs;
}

GpxTrackSegment& GpxFile::operator[](int n) {
    assert(n< track_segments.size());
    return track_segments[n];
}

GpxPoint& GpxFile::operator()(int n) {
    int nn = n;
    for (int i=0; i<track_segments.size(); ++i) {
        if (track_segments[i].pointCount()>nn) {
            return track_segments[i][nn];
        } else {
            nn -= track_segments[i].pointCount();
        }
    }
    assert(nn==0);
}

void GpxFile::addTrack(const GpxTrackSegment &seg) {
    track_segments.push_back(seg);
}

void GpxFile::addPoint(const GpxPoint &pt, int track) {
    if (track==-1) {
        track = track_segments.size()-1;
    }
                
    assert(track < track_segments.size());

    track_segments[track].addPoint(pt);
}

GpxTrackSegment &GpxFile::lastSegment() {
    assert(track_segments.size()>0);
    return track_segments[track_segments.size()-1];
}
GpxPoint &GpxFile::lastPoint() {
    assert(track_segments.size()>0);
    return track_segments[track_segments.size()-1].lastPoint();
}

void GpxFile::setTime(QDateTime time) {
    _time = time;
}
QDateTime GpxFile::time() {
    return _time;
}


int GpxFile::segmentCount() {
    return track_segments.size();
}
int GpxFile::pointCount() {
    int cnt=0;
    for (int i=0; i<track_segments.size(); ++i) {
        cnt += track_segments[i].pointCount();
    }
    return cnt;
}

time_t GpxFile::duration() {
    time_t dur=0;
    for (int i=0; i<track_segments.size(); ++i) {
        dur += track_segments[i].duration();
    }
    return dur;
}

void GpxFile::purgeEmptyTracks() {
    for (int i=0; i<track_segments.size(); ++i) {
        if (track_segments[i].pointCount()==0) {
            track_segments.removeAt(i);
            --i;
        }
    }
}
    
bool GpxFile::readFile(QString fname, bool pe) {
    GpxParser handler(*this);
    QFile file( fname );
    QXmlInputSource source( &file );

    QXmlSimpleReader reader;
    reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
    reader.setContentHandler( &handler );
    reader.parse( source );

    if (pe) purgeEmptyTracks();

    return true;
}

void GpxFile::boundLatLon(double &minLat, double &minLon, double &minEle,
                          double &maxLat, double &maxLon, double &maxEle) {
    assert(track_segments.size()>0);

    double tminLat, tminLon, tminEle, tmaxLat, tmaxLon, tmaxEle;

    track_segments[0].boundLatLon(minLat, minLon, minEle, maxLat, maxLon, maxEle);

    for (int i=0; i< track_segments.size(); ++i) {

        track_segments[i].boundLatLon(tminLat, tminLon, tminEle, tmaxLat, tmaxLon, tmaxEle);

        if (tminLat < minLat) minLat = tminLat;
        if (tmaxLat > maxLat) maxLat = tmaxLat;

        if (tminLon < minLon) minLon = tminLon;
        if (tmaxLon > maxLon) maxLon = tmaxLon;
        
        if (tminEle < minEle) minEle = tminEle;
        if (tmaxEle > maxEle) maxEle = tmaxEle;
    }
}

void GpxFile::boundUTM(double &minX, double &minY, double &minEle,
                       double &maxX, double &maxY, double &maxEle) {
    assert(track_segments.size()>0);

    double tminX, tminY, tminEle, tmaxX, tmaxY, tmaxEle;

    track_segments[0].boundUTM(minX, minY, minEle, maxX, maxY, maxEle);

    for (int i=1; i< track_segments.size(); ++i) {

        track_segments[i].boundUTM(tminX, tminY, tminEle, tmaxX, tmaxY, tmaxEle);

        if (tminX < minX) minX = tminX;
        if (tmaxX > maxX) maxX = tmaxX;

        if (tminY < minY) minY = tminY;
        if (tmaxY > maxY) maxY = tmaxY;
        
        if (tminEle < minEle) minEle = tminEle;
        if (tmaxEle > maxEle) maxEle = tmaxEle;
    }
    
}
