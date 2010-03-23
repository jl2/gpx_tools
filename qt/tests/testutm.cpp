// testutm.cpp

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

#include <iostream>
#include <cmath>

#include <QObject>
#include <QtXml>
#include <QString>
#include <QList>
#include <QDateTime>
#include <GeographicLib/UTMUPS.hpp>

#include <omp.h>

// gpx_points coorespond to <trkpt> entities in GPX files
// Also stores the point in UTM
struct gpx_point {

    // Default constructor
    gpx_point(double latitude=0.0, double longitude=0.0, double elev=0.0, QDateTime timev=QDateTime()) : lat(latitude), lon(longitude), ele(elev), time(timev) {
        setLatLon(latitude, longitude);
    }
    void setLatLon(double latitude, double longitude) {
        lat = latitude;
        lon = longitude;

        double gamma;
        double k;
        GeographicLib::UTMUPS::Forward(lat, lon, zone, north, x, y, gamma, k);
    }

    // Display to qDebug()
    void show() {
        qDebug() << "( lat, lon) (" << lat << ", " << lon << ") (x, y) (" << x << ", "
                 << y << (north ? QObject::tr(" north ") : QObject::tr(" south "))
                 << "zone " << zone << " at " << time;
    }

    // Convert to an XML string
    void toXml(QString &xmlStr) {
        xmlStr += QObject::tr("<trkpt lat=\"%1\" lon=\"%2\">"
                              "<ele>%3</ele><time>%4</time>"
                              "</trkpt>")
            .arg(lat, 0, 'f', 9)
            .arg(lon, 0, 'f', 9)
            .arg(ele, 0, 'f', 9)
            .arg(time.toString(Qt::ISODate));
        
    }

    // Latitude, longitude and elevation straight from the GPX file
    double lat, lon;
    double ele;

    // UTM coordinates
    double x, y;
    bool north;
    int zone;

    // Time from the GPX file
    QDateTime time;
};

// Compute the distance between two GPX points
double distance(const gpx_point &p1, const gpx_point &p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    double dz = p1.ele - p2.ele;
    
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// A track_seg cooresponds to the <trk> and <trkseg> tags
struct track_seg {

    QString name;

    // number and track_pts are optional
    int number;
    QList<gpx_point> track_pts;

    // Default constructor
    track_seg() : name(QObject::tr("")), number(0) { }

    // Print to qDebug()
    void show() {
        qDebug() << "Track segment \"" << name << "\" numbered " << number;
        for (int i=0; i< track_pts.size(); ++i) {
            track_pts[i].show();
        }
    }

    // Convert to an XML string;
    void toXml(QString &xmlStr) {
        xmlStr += QObject::tr("<trk><name>%1</name>").arg(name);
        if (number > 0) {
            xmlStr += QObject::tr("<number>%1</number>").arg(number);
        }
        xmlStr += QObject::tr("<trkseg>");
        for (int i=0; i< track_pts.size(); ++i) {
            track_pts[i].toXml(xmlStr);
        }
        xmlStr += QObject::tr("</trkseg></trk>");
    }

    // Calculate the length of the track segment
    double length() {
        double dist = 0.0;
        for (int i=0; i< track_pts.size()-1; ++i) {
            dist += distance(track_pts[i], track_pts[i+1]);
        }
        return dist;
    }

    double maxSpeed() {
        double curMax = 0.0;
        for (int i=0; i< track_pts.size()-1; ++i) {
            uint dt = track_pts[i+1].time.toTime_t() - track_pts[i].time.toTime_t();
            double tmp = distance(track_pts[i], track_pts[i+1]) / double(dt);
            if (tmp > curMax) {
                curMax = tmp;
            }
        }
        return curMax;
    }

};

// A GPX file
struct gpx_file {
    QList<track_seg> track_segments;
    QDateTime time;

    void show() {
        qDebug() << time;
        for (int i=0; i<track_segments.size(); ++i) {
            track_segments[i].show();
        }
    }

    void toXml(QString &xmlStr) {
        xmlStr += QObject::tr("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                              "<gpx version=\"1.0\" creator=\"Whatever\" "
                              "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                              "xmlns=\"http://www.topografix.com/GPX/1/0\" "
                              "xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 "
                              "http://www.topografix.com/GPX/1/0/gpx.xsd\">");
        if (time.isValid()) {
            xmlStr += QObject::tr("<time>%1</time>").arg(time.toString(Qt::ISODate));
        }
        for (int i=0; i<track_segments.size(); ++i) {
            track_segments[i].toXml(xmlStr);
        }
        xmlStr += QObject::tr("</gpx>\n");
    }

    double length() {
        double dist = 0.0;
        for (int i=0; i< track_segments.size(); ++i) {
            dist += track_segments[i].length();
        }
        return dist;
    }

    double maxSpeed() {
        if (track_segments.size()==0) return 0.0;

        double *speeds = new double[track_segments.size()];

        for (int i=0; i<track_segments.size(); ++i) {
            speeds[i] = track_segments[i].maxSpeed();
        }
        double maxSpeed = 0.0;
        for (int i=0; i<track_segments.size(); ++i) {
            if (speeds[i] > maxSpeed) {
                maxSpeed = speeds[i];
            }
        }
        delete [] speeds;
        return maxSpeed;
    }
};

// Callback handler class required for SAX parsing with Qt
class GpxParser : public QXmlDefaultHandler {
private:
    // Keep track of current state
    union GpxState {
        struct {
            // One bit field per tag type
            unsigned in_gpx:1;
            unsigned in_trk:1;
            unsigned in_trk_seg:1;
            unsigned in_trk_pt:1;
            unsigned in_ele:1;
            unsigned in_time:1;
            unsigned in_name:1;
            unsigned in_num:1;
        } states;
        unsigned state;
    };

    GpxState curState;

    gpx_file &gpx;
    int curSeg;

    gpx_point curPoint;
    QString curVal;

public:
    // Clear out the state
    GpxParser(gpx_file &out) : gpx(out), curSeg(-1) {
        curState.state = 0;
    }
    
    bool startDocument() { 
        return true; 
    }

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
        curVal=QObject::tr("");

        // Mostly just keep track of the state
        if (name==QObject::tr("gpx")) {
            curState.states.in_gpx = 1;

        } else if (name==QObject::tr("time")) {
            curState.states.in_time = 1;
            
        } else if (name==QObject::tr("bounds")) {
            
        } else if (name==QObject::tr("trk")) {
            curState.states.in_trk = 1;

            // Add a new track segment
            gpx.track_segments.push_back(track_seg());
            ++curSeg;
            
        } else if (name==QObject::tr("name")) {
            curState.states.in_name = 1;

        } else if (name==QObject::tr("number")) {
            curState.states.in_num = 1;
            
        } else if (name==QObject::tr("trkseg")) {
            curState.states.in_trk_seg = 1;
            
        } else if (name==QObject::tr("trkpt")) {
            curState.states.in_trk_pt = 1;

            // Set the latitude and longitude of the current point
            curPoint.setLatLon(attrs.value("lat").toDouble(), attrs.value("lon").toDouble());
            
        } else if (name==QObject::tr("ele")) {
            curState.states.in_ele = 1;
        }
        return true;
    }

    bool endElement( const QString&, const QString&, const QString &name ) {

        // Also mostly just keeping track of state
        if (name==QObject::tr("gpx")) {
            curState.states.in_gpx = 0;

        } else if (name==QObject::tr("time")) {
            curState.states.in_time = 0;

            // if we're in a <trkpt> tag, this is the pt's time
            if (curState.states.in_trk_pt) {
                curPoint.time = QDateTime::fromString(curVal,Qt::ISODate);

            } else {
                // Otherwise it's the global GPX file time
                gpx.time = QDateTime::fromString(curVal,Qt::ISODate);
            }
            
        } else if (name==QObject::tr("bounds")) {
            // Ignore bounds for now
            
        } else if (name==QObject::tr("trk")) {
            curState.states.in_trk = 0;
            
        } else if (name==QObject::tr("name")) {
            curState.states.in_name = 0;

            // The segment name
            if (curState.states.in_trk) {
                gpx.track_segments[curSeg].name=curVal;
            }
            
        } else if (name==QObject::tr("number")) {
            curState.states.in_num = 0;

            // The segment number
            if (curState.states.in_trk) {
                gpx.track_segments[curSeg].number=curVal.toInt();
            }
            
        } else if (name==QObject::tr("trkseg")) {
            curState.states.in_trk_seg = 0;
            
        } else if (name==QObject::tr("trkpt")) {
            curState.states.in_trk_pt = 0;

            // The track_pt should be completely filled out now, so push it onto the current segment
            if (curState.states.in_trk_seg) {
                gpx.track_segments[curSeg].track_pts.push_back(curPoint);
            }
            
        } else if (name==QObject::tr("ele")) {
            curState.states.in_ele = 0;

            // The pt's elevation
            curPoint.ele = curVal.toDouble();
        }
        curVal = QObject::tr("");
        return true;
    }
};

bool readFile(QString fname, gpx_file &gpx) {
    GpxParser handler(gpx);
    QFile file( fname );
    QXmlInputSource source( &file );

    QXmlSimpleReader reader;
    reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
    reader.setContentHandler( &handler );
    reader.parse( source );
    return true;
}

double meter2mile(double len) {
    return len * 0.000621371192;
}
double meterPerSecond2MilePerHour(double speed) {
    return speed * 2.23693629;
}
int main(int argc ,char *argv[]) {

    if (argc != 3) {
        qDebug() << "No input or output file name given!";
        return 1;
    }
    gpx_file gpx;

    readFile(argv[1], gpx);
    
    qDebug() << "Total distance: " << meter2mile(gpx.length());
    qDebug() << "Number of segments: " << gpx.track_segments.size();
    qDebug() << "GPX File max speed: " << meterPerSecond2MilePerHour(gpx.maxSpeed()) << " mph";

    for (int i=0; i<gpx.track_segments.size(); ++i) {
        qDebug() << "Segment " << (i+1) << " has " <<
            gpx.track_segments[i].track_pts.size() << " points, "
            "length " << gpx.track_segments[i].length() <<
            " and max speed of " << meterPerSecond2MilePerHour(gpx.track_segments[i].maxSpeed()) << " mph";
    }

    QString theData;
    gpx.toXml(theData);
    
    QFile outFile(argv[2]);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Could not open " << argv[2] << " for writing!";
    }

    QTextStream out(&outFile);
    out << theData;

    return 0;
}
