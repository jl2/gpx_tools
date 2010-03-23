// gpxgui.h

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

#ifndef GPXGUI_INCLUDE_H
#define GPXGUI_INCLUDE_H

#include <QMainWindow>

class QToolBar;
class QMenu;
class QAction;
class QLabel;
class QSplitter;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QFrame;

class GpxGui : public QMainWindow {
  Q_OBJECT;

  QToolBar *toolBar;
  QMenu *mainMenu;
  QMenu *solverMenu;
 public:
  GpxGui(QWidget *parent = 0);
  ~GpxGui();
  
  public slots:
  void openFile();
  void closeFile();
  void about();

 private:
  void readSettings();
  void setupActions();
  void setupFileActions();
  void setupEditActions();
  void setupCallPlanActions();
  void setupToolsActions();
  void setupHelpActions();
  void fillInAction(QAction **action, QString text,
		    QString toolTip, const char *method,
		    Qt::ConnectionType type, QIcon icon);
  void fillInAction(QAction **action, QString text,
		    QString toolTip, const char *method,
		    QIcon icon);


  void setupToolBar();
  void setupMenuBar();
  void setupStatusBar();

  void openFileError(QString what);

  void disableActionsOnClose();
  void enableActionsOnOpen();
  void notYetImplemented();

  bool canContinue();

  void doRedraw();
  
  QString openDir;
  QString curFileName;
  QString titleBarPrefix;
  
  QLabel *curDistanceLbl;
  QLabel *curFileNameLbl;
  
  QAction *openAction;
  QAction *closeAction;
  QAction *exitAction;
  QAction *aboutAction;
  QAction *configAction;
};

#endif
