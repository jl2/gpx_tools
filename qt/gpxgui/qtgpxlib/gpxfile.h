// gpxfile.h

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

#ifndef GPX_FILE_H
#define GPX_FILE_H

#include "gpxtracksegment.h"
#include "gpxpoint.h"

#include "gpxelement.h"
#include "track.h"

#include <QList>
#include <QDateTime>
#include <QMap>
#include <QString>

#include <QtXml>

class GpxFile : public GpxElement, public Track {
public:
    GpxFile(QString fname, bool purgeEmpty = true);
    
    void toXml(QString &xmlStr);

    double length();

    double maxSpeed();

    GpxTrackSegment& operator[](int n);

    GpxPoint &operator()(int n);

    void addTrack(const GpxTrackSegment &seg);
    void addPoint(const GpxPoint &pt, int track=-1);

    GpxTrackSegment &lastSegment();
    GpxPoint &lastPoint();

    void setTime(QDateTime time);
    QDateTime time();

    int segmentCount();
    int pointCount();
    time_t duration();
    void purgeEmptyTracks();

    void removeTrack(int idx);

    void boundLatLon(double &minLat, double &minLon, double &minEle,
                     double &maxLat, double &maxLon, double &maxEle);

    void boundUTM(double &minX, double &minY, double &minEle,
                  double &maxX, double &maxY, double &maxEle);

private:
    QList<GpxTrackSegment> track_segments;
    QDateTime _time;

    // Callback handler class required for SAX parsing with Qt
    class GpxParser : public QXmlDefaultHandler {
    private:
        // Keep track of current state
        QMap<QString, bool> curState;

        GpxPoint curPoint;

        QString curVal;

        double clat, clon, cele;
        QDateTime ctime;
        GpxFile &gpx;

    public:
        // Clear out the state
        GpxParser(GpxFile &file) : gpx(file) { }
    
        // Character data can be reported in multiple calls
        // For example <tag>character data</tag>
        // could call characters("character"), characters(" data")
        // so save the data until the end tag
        bool characters( const QString & ch ) {
            curVal += ch;
            return true;
        }

        // Called for each opening tag
        bool startElement( const QString&, const QString&, const QString &name, const QXmlAttributes &attrs ) {

            // Clear the character data
            curVal = "";

            // Keep track of state
            curState[name] = true;

            if (name == "trk") {
                // Add a new track segment
                gpx.addTrack(GpxTrackSegment());
            
            } else if (name == "trkpt") {
                // Set the latitude and longitude of the current point
                clat = attrs.value("lat").toDouble();
                clon = attrs.value("lon").toDouble();
            }

            return true;
        }

        bool endElement( const QString&, const QString&, const QString &name ) {
            
            // Also mostly just keeping track of state
            if (name == "time") {

                if (curState["trkpt"]) {
                    ctime = QDateTime::fromString(curVal,Qt::ISODate);

                } else {
                    gpx.setTime(QDateTime::fromString(curVal,Qt::ISODate));
                }
            
            } else if (name == "name") {
                // The segment name
                if (curState["trk"]) {
                    gpx.lastSegment().setName(curVal);
                }
            
            } else if (name == "number") {
                // The segment number
                if (curState["trk"]) {
                    gpx.lastSegment().setNumber(curVal.toInt());
                }
            
            } else if (name == "trkpt") {
                if (curState["trkseg"]) {
                    gpx.addPoint(GpxPoint(clat, clon, cele, ctime));
                }

            } else if (name == "ele") {
                cele = curVal.toDouble();
            }

            curState[name] = false;

            return true;
        }
    };
    
    bool readFile(QString fname, bool purge);
};

#endif
