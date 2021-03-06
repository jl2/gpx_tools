// gpxgui.cpp

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

#include <QtGui>

#include <stdexcept>
#include "gpxgui.h"
#include "gpxtreewidget.h"
#include "gpxfile.h"

#include "elevationwidget.h"

void GpxGui::readSettings() {
}

GpxGui::GpxGui(QWidget *parent) : QMainWindow(parent), gpx(0) {
    readSettings();
    setupActions();
    setupToolBar();
    setupMenuBar();
    setupStatusBar();

    titleBarPrefix = tr("GpxGui");
    setWindowTitle(titleBarPrefix);

    split = new QSplitter;
    gpxTree = new GpxTreeWidget;
    visTabs = new QTabWidget;
    eleW = new ElevationWidget;
    visTabs->insertTab(0, eleW, tr("Elevation Profile"));

    split->addWidget(gpxTree);
    split->addWidget(visTabs);
    connect(gpxTree, SIGNAL(gpxChanged()),
            eleW, SLOT(gpxChanged()));
    // split->addWidget(new QLabel("Visualization widget goes here..."));
    setCentralWidget(split);
}

GpxGui::~GpxGui() {
    if (gpx) delete gpx;
}

void GpxGui::fillInAction(QAction **action, QString text,
                          QString toolTip, const char *method,
                          Qt::ConnectionType type, QIcon icon = QIcon(":/images/blank.png")) {
    *action = new QAction(this);
    (*action)->setText(text);
    (*action)->setToolTip(toolTip);
    (*action)->setStatusTip(toolTip);
    (*action)->setIcon(icon);
    (*action)->setDisabled(true);
    connect(*action, SIGNAL(triggered()), this, method, type);
}
void GpxGui::fillInAction(QAction **action, QString text,
                          QString toolTip, const char *method,
                          QIcon icon = QIcon(":/images/blank.png")) {
    *action = new QAction(this);
    (*action)->setText(text);
    (*action)->setToolTip(toolTip);
    (*action)->setStatusTip(toolTip);
    (*action)->setIcon(icon);
    (*action)->setDisabled(true);
    connect(*action, SIGNAL(triggered()), this, method);
}
void GpxGui::setupFileActions() {
    fillInAction(&openAction, tr("Open..."), tr("Open an GPX file."),
                 SLOT(openFile()), QIcon(":/images/open.png"));

    fillInAction(&saveAction, tr("Save"), tr("Save changes to GPX file."),
                 SLOT(saveFile()), QIcon(":/images/save.png"));

    fillInAction(&saveAsAction, tr("Save As..."), tr("Save GPX file to a new file."),
                 SLOT(saveAsFile()), QIcon(":/images/save.png"));

    fillInAction(&closeAction, tr("Close"), tr("Close current GPX file."),
                 SLOT(closeFile()), QIcon(":/images/close.png"));

    fillInAction(&exitAction, tr("E&xit"), tr("Exit GpxGui"), SLOT(close()));
}

void GpxGui::setupHelpActions() {
    fillInAction(&aboutAction, tr("&About"), tr("About GpxGui"),
                 SLOT(about()), QIcon(":/images/globe.png"));

}

void GpxGui::setupActions() {
    setupFileActions();
    setupHelpActions();

    aboutAction->setDisabled(false);
    openAction->setDisabled(false);
    exitAction->setDisabled(false);
}

void GpxGui::setupToolBar() {
    QToolBar *tb = new QToolBar("Main", this);
    tb->addAction(openAction);
    tb->addAction(saveAsAction);
    tb->addAction(closeAction);
    addToolBar(tb);
}

void GpxGui::setupMenuBar() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addAction(closeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    menuBar()->addSeparator();
  
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
}

void GpxGui::setupStatusBar() {
    statusBar()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    curDistanceLbl = new QLabel;
    curDistanceLbl->setMaximumWidth(fontMetrics().maxWidth()*24);
    curDistanceLbl->setMinimumWidth(fontMetrics().maxWidth()*10);
    curDistanceLbl->setText("Current Distance");
    curDistanceLbl->setAlignment(Qt::AlignHCenter);

    curFileNameLbl = new QLabel;
    curFileNameLbl->setMaximumWidth(fontMetrics().maxWidth()*24);
    curFileNameLbl->setMinimumWidth(fontMetrics().maxWidth()*10);
    curFileNameLbl->setText("File Name");
    curFileNameLbl->setAlignment(Qt::AlignHCenter);
  
    statusBar()->addWidget(curDistanceLbl);
    statusBar()->addWidget(curFileNameLbl);
}
void GpxGui::notYetImplemented() {
    QMessageBox::critical(this, tr("Not Yet Implemented"),
                          tr("Oops! That feature is not yet implemented!"),
                          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

bool GpxGui::canContinue() {
    return true;
}
void GpxGui::openFile() {
    if (!canContinue()) {
        return;
    }
    QString newFileName = QFileDialog::getOpenFileName(this,
                                                       tr("Choose a file to open"),
                                                       openDir,
                                                       tr("GPX Files (*.gpx)"));
    if (newFileName == tr("")) {
        // Cancelled
        return;
    }

    if (gpx) delete gpx;

    gpx = new GpxFile(newFileName, true);
    curFileName = newFileName;

    ((GpxTab*)visTabs->currentWidget())->setGpx(gpx);

    enableActionsOnOpen();

    gpxTree->setGpxFile(gpx);
}

void GpxGui::updateUI() {
  curFileNameLbl->setText(curFileName);
  curDistanceLbl->setText(tr("%1 meters").arg(gpx->length()));

  setWindowTitle(tr("%1 - %2").arg(titleBarPrefix).arg(curFileName));
}
void GpxGui::openFileError(QString what) {
    QMessageBox::critical(this, tr("Failed"),
                          tr("Oops! That's a bad GPX file:\n%1").arg(what),
                          QMessageBox::Ok, QMessageBox::NoButton,
                          QMessageBox::NoButton);
}

void GpxGui::disableActionsOnClose() {
    closeAction->setDisabled(true);
    saveAction->setDisabled(true);
    saveAsAction->setDisabled(true);
}
void GpxGui::enableActionsOnOpen() {
    closeAction->setDisabled(false);
    saveAction->setDisabled(false);
    saveAsAction->setDisabled(false);
}

void GpxGui::closeFile() {
    if (!canContinue()) {
        return;
    }
    disableActionsOnClose();
    setWindowTitle(titleBarPrefix);

    curFileName = tr("");
    curFileNameLbl->setText(curFileName);
    curDistanceLbl->setText(tr("0 meters"));

    if (gpx) delete gpx;
    gpx = 0;
    gpxTree->setGpxFile(gpx);
    ((GpxTab*)visTabs->currentWidget())->setGpx(gpx);
}

void GpxGui::about() {
    QMessageBox::about(this,
                       tr("About"),
                       tr("<h2>GpxGui</h2>"
                          "<p>By Jeremiah LaRocco.</p>"));
}

void GpxGui::saveFile() {
  if (gpx==0) return;

  QString strValue;
  gpx->toXml(strValue);

  QFile file( curFileName );
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    out << strValue;
  }
}

void GpxGui::saveAsFile() {
  if (gpx==0) return;
  QString newFileName = QFileDialog::getSaveFileName(this,
						     tr("Choose a file to save to"),
						     openDir,
						     tr("GPX Files (*.gpx)"));
  if (newFileName == tr("")) return;
  curFileName = newFileName;
    

  QString strValue;
  gpx->toXml(strValue);

  QFile file( newFileName );
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    out << strValue;
  }
  updateUI();
}
