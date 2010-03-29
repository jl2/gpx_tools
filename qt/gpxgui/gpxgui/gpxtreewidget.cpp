// gpxtreewidget.cpp

// Copyright (c) 2010, Jeremiah LaRocco jlarocco@jlarocco.com

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

#include <QtGui>

#include "gpxtreewidget.h"

#include "gpxfile.h"
#include "gpxtracksegment.h"

#include "unitconversion.h"

GpxTreeWidget::GpxTreeWidget(GpxFile *gpx) : _gpx(gpx) {
    QLayout *mainLayout = new QHBoxLayout;

    _gpxTree = new QTreeWidget;
    _gpxTree->setColumnCount(3);
    _gpxTree->setHeaderLabels(QStringList() << tr("Element") << tr("Length (miles)") << tr("# Pts") << tr("Duration") << tr("Max Speed (mph)") << tr("Avg. Speed (mph)"));
    for (int i=0; i<6; ++i) {
        _gpxTree->header()->setResizeMode(i, QHeaderView::Stretch);
    }

    if (_gpx) {
        buildTree();
    }
    
    mainLayout->addWidget(_gpxTree);
    setLayout(mainLayout);
}

void GpxTreeWidget::buildTree() {

    _gpxTree->clear();

    if (_gpx==0) return;

    QTreeWidgetItem *gpxfile = new QTreeWidgetItem(_gpxTree);
    gpxfile->setText(0, tr("GpxFile"));
    gpxfile->setText(1, tr("%1").arg(meter2mile(_gpx->length())));
    gpxfile->setText(2, tr("%1").arg(_gpx->pointCount()));
    gpxfile->setText(3, tr("%1").arg(formatDuration(_gpx->duration(), true)));
    gpxfile->setText(4, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->maxSpeed())));
    gpxfile->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->averageSpeed())));

    for (int i=0; i<_gpx->segmentCount(); ++i) {
        QTreeWidgetItem *track = new QTreeWidgetItem(gpxfile);
        GpxTrackSegment cur = (*_gpx)[i];
        track->setText(0, tr("%1 (Track %2)").arg(cur.name()).arg(cur.number()));
        track->setText(1, tr("%1").arg(meter2mile(cur.length())));
        track->setText(2, tr("%1").arg(cur.pointCount()));
        track->setText(3, tr("%1").arg(formatDuration(cur.duration(), true)));
        track->setText(4, tr("%1").arg(meterPerSecond2MilePerHour(cur.maxSpeed())));
        track->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(cur.averageSpeed())));
    }
}

void GpxTreeWidget::setGpxFile(GpxFile *gpx) {
    _gpx = gpx;
    buildTree();
}
