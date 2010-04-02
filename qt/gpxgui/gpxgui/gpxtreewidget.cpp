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
#include <cassert>

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
    setSelectionMode(QAbstractItemView::ExtendedSelection);


    mergeAction = new QAction(this);
    mergeAction->setText(tr("Merge"));
    mergeAction->setToolTip(tr("Merge the selected tracks into one."));
    mergeAction->setDisabled(true);

    removeAction = new QAction(this);
    removeAction->setText(tr("Remove"));
    removeAction->setToolTip(tr("Remove the selected tracks."));
    removeAction->setDisabled(true);

    splitAction = new QAction(this);
    splitAction->setText(tr("Split"));
    splitAction->setToolTip(tr("Split selected track into seperate file."));
    splitAction->setDisabled(true);

    multiContextMenu = new QMenu(tr("Multi-selection Track Menu"));
    multiContextMenu->addAction(mergeAction);
    multiContextMenu->addAction(removeAction);
    
    singleContextMenu = new QMenu(tr("Single selection Track Menu"));
    singleContextMenu->addAction(splitAction);
    singleContextMenu->addAction(removeAction);

    connect(mergeAction, SIGNAL(triggered()), this, SLOT(mergeTracks()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeTracks()));
    connect(splitAction, SIGNAL(triggered()), this, SLOT(splitTrack()));

    if (_gpx) {
        buildTree();
    }
}

void GpxTreeWidget::buildTree() {

    clear();

    removeAction->setEnabled(false);
    mergeAction->setEnabled(false);
    splitAction->setEnabled(false);

    if (_gpx==0) return;

    root = new QTreeWidgetItem(this);
    for (int i=0; i<_gpx->segmentCount(); ++i) {
        new QTreeWidgetItem(root);
    }
    expandItem(root);
    recompute();
    removeAction->setEnabled(true);
    mergeAction->setEnabled(true);
    splitAction->setEnabled(true);
}

void GpxTreeWidget::setGpxFile(GpxFile *gpx) {
    _gpx = gpx;
    buildTree();
}

void GpxTreeWidget::contextMenuEvent(QContextMenuEvent *event) {
    if (_gpx) {
        if (selectedItems().size()==1) {
            singleContextMenu->popup(mapToGlobal(event->pos()));
        } else {
            multiContextMenu->popup(mapToGlobal(event->pos()));
        }
    }
}

void GpxTreeWidget::mergeTracks() {
    assert(_gpx!=0);

    QList<QTreeWidgetItem*> tracks = selectedItems();
    tracks.removeAll(root);
    if (tracks.size()==0) return;
    QList<QString> toMerge;

    toMerge.push_back(tracks[0]->text(1));
    for (int i=1; i<tracks.size(); ++i) {
        toMerge.push_back(tracks[i]->text(1));
        delete tracks[i];
    }
    _gpx->mergeTracksByName(toMerge);
    recompute();
}

void GpxTreeWidget::removeTracks() {
    assert(_gpx!=0);
    QList<QTreeWidgetItem*> tracks = selectedItems();
    tracks.removeAll(root);
    if (tracks.size()==0) return;

    QList<QString> toRemove;

    for (int i=0; i<tracks.size(); ++i) {
        toRemove.push_back(tracks[i]->text(1));
        delete tracks[i];
    }
    _gpx->removeTracksByName(toRemove);
    recompute();
}

void GpxTreeWidget::splitTrack() {
    assert(_gpx!=0);
    assert(selectedItems().size()==1);
    
    QList<QTreeWidgetItem*> tracks = selectedItems();
    tracks.removeAll(root);
    if (tracks.size()==0) return;

    QString newFileName = QFileDialog::getSaveFileName(this,
                                                       tr("Choose a file to save to"),
                                                       tr("."),
                                                       tr("GPX Files (*.gpx)"));
    if (newFileName == tr("")) return;
    GpxTrackSegment s = _gpx->segmentByName(selectedItems()[0]->text(1));
    GpxFile *newGpx = new GpxFile(s);
    QString strValue;
    newGpx->toXml(strValue);

    QFile file( newFileName );
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream out(&file);
        out << strValue;
    }
    delete newGpx;
}

void GpxTreeWidget::recompute() {
    if (root) {
        root->setText(0, tr("GpxFile"));
        root->setText(2, tr("%1").arg(meter2mile(_gpx->length())));
        root->setText(3, tr("%1").arg(_gpx->pointCount()));
        root->setText(4, tr("%1").arg(formatDuration(_gpx->duration(), true)));
        root->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->maxSpeed())));
        root->setText(6, tr("%1").arg(meterPerSecond2MilePerHour(_gpx->averageSpeed())));

        QTreeWidgetItem *track = root;
        
        for (int i=0; i<_gpx->segmentCount(); ++i) {
            track = itemBelow(track);
            if (track == 0) return;
            GpxTrackSegment cur = (*_gpx)[i];
            track->setText(0, tr("Track %1").arg(cur.number()));

            track->setText(1, tr("%1").arg(cur.name()));
            track->setText(2, tr("%1").arg(meter2mile(cur.length())));
            track->setText(3, tr("%1").arg(cur.pointCount()));
            track->setText(4, tr("%1").arg(formatDuration(cur.duration(), true)));
            track->setText(5, tr("%1").arg(meterPerSecond2MilePerHour(cur.maxSpeed())));
            track->setText(6, tr("%1").arg(meterPerSecond2MilePerHour(cur.averageSpeed())));
        }
    }
    
}
