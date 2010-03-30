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
    // _gpxTree = new QTreeWidget;
    QStringList columns = QStringList()
        << tr("Track #")
        << tr("Name")
        << tr("Length (miles)")
        << tr("# Pts")
        << tr("Duration")
        << tr("Max Speed (mph)")
        << tr("Avg. Speed (mph)");

    setColumnCount(columns.size());
    
    setHeaderLabels(columns);
    for (int i=0; i<6; ++i) {
        header()->setResizeMode(i, QHeaderView::Stretch);
    }
    setSelectionMode(QAbstractItemView::ContiguousSelection);

    contextMenu = new QMenu(tr("Track Menu"));
    mergeAction = new QAction(this);
    mergeAction->setText(tr("Merge"));
    mergeAction->setToolTip(tr("Merge the selected tracks into one."));
    mergeAction->setDisabled(true);
    contextMenu->addAction(mergeAction);

    removeAction = new QAction(this);
    removeAction->setText(tr("Remove"));
    removeAction->setToolTip(tr("Remove the selected tracks."));
    removeAction->setDisabled(true);
    contextMenu->addAction(removeAction);

    connect(mergeAction, SIGNAL(triggered()), this, SLOT(mergeTracks()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeTracks()));
    if (_gpx) {
        buildTree();
    }
}

void GpxTreeWidget::buildTree() {

    clear();

    removeAction->setEnabled(false);
    mergeAction->setEnabled(false);

    if (_gpx==0) return;

    QTreeWidgetItem *gpxfile = new QTreeWidgetItem(this);
    
    gpxfile->setText(0, tr("GpxFile"));
    gpxfile->setText(2, tr("%1").arg(meter2mile(_gpx->length())));
    gpxfile->setText(3, tr("%1").arg(_gpx->pointCount()));
    gpxfile->setText(4, tr("%1").arg(formatDuration(_gpx->duration(), true)));
    gpxfile->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->maxSpeed())));
    gpxfile->setText(6, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->averageSpeed())));

    for (int i=0; i<_gpx->segmentCount(); ++i) {
        QTreeWidgetItem *track = new QTreeWidgetItem(gpxfile);
        GpxTrackSegment cur = (*_gpx)[i];
        track->setText(0, tr("Track %1").arg(cur.number()));

        track->setText(1, tr("%1").arg(cur.name()));
        track->setText(2, tr("%1").arg(meter2mile(cur.length())));
        track->setText(3, tr("%1").arg(cur.pointCount()));
        track->setText(4, tr("%1").arg(formatDuration(cur.duration(), true)));
        track->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(cur.maxSpeed())));
        track->setText(6, tr("%1").arg(meterPerSecond2MilePerHour(cur.averageSpeed())));
    }
    removeAction->setEnabled(true);
    mergeAction->setEnabled(true);
}

void GpxTreeWidget::setGpxFile(GpxFile *gpx) {
    _gpx = gpx;
    buildTree();
}

void GpxTreeWidget::contextMenuEvent(QContextMenuEvent *event) {
    if (_gpx) {
        contextMenu->popup(mapToGlobal(event->pos()));
    }
}

void GpxTreeWidget::mergeTracks() {
}

void GpxTreeWidget::removeTracks() {
    if (_gpx==0) return;
    QList<QTreeWidgetItem*> tracks = selectedItems();

    QList<QString> toRemove;

    QRegExp trackRE("Track ([0-9]+)");
    for (int i=0; i<tracks.size(); ++i) {
        toRemove.push_back(tracks[i]->text(1));
        delete tracks[i];
    }

    for (int i=0; i<toRemove.size(); ++i) {
        _gpx->removeTrackByName(toRemove[i]);
    }
}

void GpxTreeWidget::recompute() {
    
}
