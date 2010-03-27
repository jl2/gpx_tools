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

#include <QtXml>
#include <QString>
#include <QList>
#include <QDateTime>

#include <GeographicLib/UTMUPS.hpp>

#include <cassert>
#include <cmath>

class GpxElement {
public:
    GpxElement() {}
    virtual void toXml(QString &xmlStr)=0;
};

class GpxPoint : public GpxElement {
public:
    // Default constructor
    GpxPoint(double latitude=0.0, double longitude=0.0, double elev=0.0, QDateTime timev=QDateTime()) : _lat(latitude), _lon(longitude), _ele(elev), _time(timev) {
        setLatLon(latitude, longitude);
    }
    void setLatLon(double latitude, double longitude) {
        _lat = latitude;
        _lon = longitude;

        double gamma;
        double k;
        GeographicLib::UTMUPS::Forward(_lat, _lon, _zone, _north, _x, _y, gamma, k);
    }

    // Convert to an XML string
    void toXml(QString &xmlStr) {
        xmlStr += QString("<trkpt lat=\"%1\" lon=\"%2\">"
                          "<ele>%3</ele><time>%4</time>"
                          "</trkpt>")
            .arg(_lat, 0, 'f', 9)
            .arg(_lon, 0, 'f', 9)
            .arg(_ele, 0, 'f', 9)
            .arg(_time.toString(Qt::ISODate));
        
    }
    // Compute the distance between two GPX points
    double distanceTo(const GpxPoint &p2) {
        double dx = _x - p2._x;
        double dy = _y - p2._y;
        double dz = _ele - p2._ele;
    
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }

    double speedBetween(const GpxPoint &p2) {
        double dist = distanceTo(p2);
        double dt = p2._time.toTime_t() - _time.toTime_t();
        return dist/dt;
    }

    time_t secondsBetween(const GpxPoint &p2) {
        return p2._time.toTime_t() - _time.toTime_t();
    }

    double latitude() {
        return _lat;
    }
    double longitude() {
        return _lon;
    }

    double elevation() {
        return _ele;
    }

    QDateTime time() {
        return _time;
    }

    double x() {
        return _x;
    }
    double y() {
        return _y;
    }

    bool north() {
        return _north;
    }
    int zone() {
        return _zone;
    }

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
protected:
    Track() { }
    Track& operator =(const Track&) { return *this;}
};

class GpxTrackSegment : public GpxElement, public Track {
public:
    GpxTrackSegment() : _name(""), _number(0) { }

    // Convert to an XML string;
    void toXml(QString &xmlStr) {
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
    double length() {
        double dist = 0.0;
        for (int i=0; i< track_pts.size()-1; ++i) {
            dist += track_pts[i].distanceTo(track_pts[i+1]);
        }
        return dist;
    }

    time_t duration() {
        if (track_pts.size()<2) return 0;
        return track_pts[0].secondsBetween(track_pts[track_pts.size()-1]);
    }

    double maxSpeed() {
        double curMax = 0.0;
        for (int i=0; i< track_pts.size()-1; ++i) {

            double spd = track_pts[i].speedBetween(track_pts[i+1]);

            if (spd > curMax) {
                curMax = spd;
            }
        }
        return curMax;
    }

    GpxPoint& operator [](int n) {
        assert(n<track_pts.size());

        return track_pts[n];
    }

    void addPoint(const GpxPoint &pt) {
        track_pts.push_back(pt);
    }

    GpxPoint &lastPoint() {
        assert(track_pts.size()>0);
        return track_pts[track_pts.size()-1];
    }

    QString name() {
        return _name;
    }
    void setName(const QString &name) {
        _name = name;
    }
    
    int number() {
        return _number;
    }
    void setNumber(int number) {
        _number = number;
    }

    int pointCount() {
        return track_pts.size();
    }
    
private:
    QString _name;

    // number and track_pts are optional
    int _number;
    QList<GpxPoint> track_pts;
};

class GpxFile : public GpxElement, public Track {
public:
    GpxFile(QString fname) : _time(QDateTime()) {
        readFile(fname);
    }
    
    void toXml(QString &xmlStr) {
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

    double length() {
        double dist = 0.0;
        for (int i=0; i< track_segments.size(); ++i) {
            dist += track_segments[i].length();
        }
        return dist;
    }

    double maxSpeed() {
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

    GpxTrackSegment& operator [](int n) {
        assert(n< track_segments.size());
        return track_segments[n];
    }

    void addTrack(const GpxTrackSegment &seg) {
        track_segments.push_back(seg);
    }

    void addPoint(const GpxPoint &pt, int track=-1) {
        if (track==-1) {
            track = track_segments.size()-1;
        }
                
        assert(track < track_segments.size());

        track_segments[track].addPoint(pt);
    }

    GpxTrackSegment &lastSegment() {
        assert(track_segments.size()>0);
        return track_segments[track_segments.size()-1];
    }
    GpxPoint &lastPoint() {
        assert(track_segments.size()>0);
        return track_segments[track_segments.size()-1].lastPoint();
    }

    void setTime(QDateTime time) {
        _time = time;
    }

    int segmentCount() {
        return track_segments.size();
    }
    int pointCount() {
        int cnt=0;
        for (int i=0; i<track_segments.size(); ++i) {
            cnt += track_segments[i].pointCount();
        }
        return cnt;
    }

    time_t duration() {
        time_t dur=0;
        for (int i=0; i<track_segments.size(); ++i) {
            dur += track_segments[i].duration();
        }
        return dur;
    }

    void purgeEmpty() {
        for (int i=0; i<track_segments.size(); ++i) {
            if (track_segments[i].pointCount()==0) {
                track_segments.removeAt(i);
                --i;
            }
        }
    }

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
    
    bool readFile(QString fname) {
        GpxParser handler(*this);
        QFile file( fname );
        QXmlInputSource source( &file );

        QXmlSimpleReader reader;
        reader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
        reader.setContentHandler( &handler );
        reader.parse( source );
        purgeEmpty();
        return true;
    }

};

double meter2mile(double len) {
    return len * 0.000621371192;
}
double meterPerSecond2MilePerHour(double speed) {
    return speed * 2.23693629;
}

QString formatSeconds(time_t dur) {
    int secs = dur % 60;
    int mins = (dur/60)%60;
    int hrs = dur/(60*60);
    QChar z('0');
    if (hrs>0) {
        return QString("%1:%2:%3").arg(hrs,2,10,z).arg(mins,2,10,z).arg(secs,2,10,z);
    } else if (mins>0) {
        return QString("%1:%2").arg(mins,2,10,z).arg(secs,2,10,z);
    }
    return QString("%1").arg(secs,2L,10,z);
}
int main(int argc ,char *argv[]) {

    if (argc != 3) {
        qDebug() << "No input or output file name given!";
        return 1;
    }
    GpxFile gpx(argv[1]);

    qDebug() << "Total distance: " << meter2mile(gpx.length()) << " miles";
    qDebug() << "Total duration: " << formatSeconds(gpx.duration());
    qDebug() << "Max speed: " << meterPerSecond2MilePerHour(gpx.maxSpeed()) << " mph";
    qDebug() << "Average speed: " << meterPerSecond2MilePerHour(gpx.averageSpeed()) << " mph";

    qDebug() << "\nNumber of segments: " << gpx.segmentCount();

    for (int i=0; i<gpx.segmentCount(); ++i) {
        qDebug() << "Segment " << (i+1) << " has " <<
            gpx[i].pointCount() << " points, "
            "length " << meter2mile(gpx[i].length()) << " miles, "
            "duration " << formatSeconds(gpx[i].duration()) <<
            "max speed of " << meterPerSecond2MilePerHour(gpx[i].maxSpeed()) << " mph, "
            "and average speed of " << meterPerSecond2MilePerHour(gpx[i].averageSpeed()) << " mph";
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
