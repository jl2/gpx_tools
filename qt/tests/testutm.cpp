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
#include <QFile>
#include <QTextStream>

#include <cassert>
#include <cmath>

#include "gpxfile.h"

double meter2mile(double len) {
    return len * 0.000621371192;
}
double meterPerSecond2MilePerHour(double speed) {
    return speed * 2.23693629;
}

QString formatDuration(time_t dur) {
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
    qDebug() << "Total duration: " << formatDuration(gpx.duration());
    qDebug() << "Max speed: " << meterPerSecond2MilePerHour(gpx.maxSpeed()) << " mph";
    qDebug() << "Average speed: " << meterPerSecond2MilePerHour(gpx.averageSpeed()) << " mph";

    qDebug() << "\nNumber of segments: " << gpx.segmentCount();

    for (int i=0; i<gpx.segmentCount(); ++i) {
        qDebug() << "Segment " << (i+1) << " has " <<
            gpx[i].pointCount() << " points, "
            "length " << meter2mile(gpx[i].length()) << " miles, "
            "duration " << formatDuration(gpx[i].duration()) <<
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
