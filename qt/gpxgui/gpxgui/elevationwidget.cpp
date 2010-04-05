// elevationwidget.cpp

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

#include "elevationwidget.h"
#include "utils.h"

ElevationWidget::ElevationWidget(QWidget *parent) : GpxTab(parent), _gpx(0) {
}

ElevationWidget::~ElevationWidget() {
}

void ElevationWidget::setGpx(GpxFile *gpx) {
    this->_gpx = gpx;
    update(rect());
}

void ElevationWidget::paintEvent(QPaintEvent *event) {
    if (_gpx == 0) return;

    QPainter p(this);
    int nPts = _gpx->pointCount();
    double minX;
    double minY;
    double minEle;
    double maxX;
    double maxY;
    double maxEle;
    int xBorder = width()*0.05;
    int yBorder = height()*0.05;
    p.setWindow(-xBorder, -yBorder, width()+xBorder, height()+yBorder);
    _gpx->boundUTM(minX, minY, minEle, maxX, maxY, maxEle);

    double de = maxEle - minEle;
    double cx = 0.0;
    double dx = double(width())/double(nPts);
    double ocx;

    while (colors.size() < _gpx->segmentCount()) {
        colors.push_back(randColor());
    }
    for (int i=0; i<_gpx->segmentCount(); ++i) {
        ocx = cx;
        QPainterPath ep;
        ep.moveTo(cx,height());
        GpxTrackSegment cs = _gpx->track(i);

        for (int j=0; j<cs.pointCount(); ++j) {
            GpxPoint tmp = cs[j];
            double ele = height()-height()*(tmp.elevation()-minEle)/de;
        
            ep.lineTo(cx, ele);
            cx += dx;
        }
        ep.lineTo(cx, height());
        ep.lineTo(ocx,height());
        p.fillPath(ep, colors[i]);
    }
}

void ElevationWidget::resizeEvent(QResizeEvent *event) {
}

void ElevationWidget::gpxChanged() {
    update();
}
