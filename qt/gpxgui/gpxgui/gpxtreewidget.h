// gpxtreewidget.h

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

#ifndef GPX_TREE_WIDGET_H
#define GPX_TREE_WIDGET_H

#include <QWidget>

#include <QTreeWidget>

class GpxFile;
class QMenu;
class QAction;

class QTreeWidgetItem;

class GpxTreeWidget : public QTreeWidget {
    Q_OBJECT;

public:
    GpxTreeWidget(GpxFile *gpx = 0);
    void setGpxFile(GpxFile *gpx);
public slots:
    void mergeTracks();
    void removeTracks();
    void splitTrack();

       signals:
    void gpxChanged();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    GpxFile *_gpx;
    QMenu *multiContextMenu;
    QMenu *singleContextMenu;

    QAction *mergeAction;
    QAction *removeAction;
    QAction *splitAction;

    void buildTree();
    void recompute();

    QTreeWidgetItem *root;
    QTreeWidgetItem *children;
};

#endif
