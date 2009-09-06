/*SDOC:**********************************************************************

	File:			mainwindow.h

	Action:		Declaration of the MainWindow window.

	Copyright © 2009, Ian Prest
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions 
	are met:
	
	1. Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright 
	notice, this list of conditions and the following disclaimer in the 
	documentation and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************:EDOC*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui { class MainWindow; }
class AddComics;

/////////////////////////////////////////////////////////////////////////////
// MainWindow window class
/////////////////////////////////////////////////////////////////////////////
class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget* parent = 0);
  ~MainWindow();
protected:
	void closeEvent(QCloseEvent* event);

signals:
	void connected(bool isConnected);

private slots:;
  void closeDatabase();
  bool openDatabase();
  bool newDatabase();
	void addComics();
	void addCustomSeries();
	void addCustomIssue();
	void about();
	void addItems(const QList<int>& items);

	void setCoverId(int id);

private:
  bool createDatabase(const QString& filename);
  bool connectDatabase(const QString& filename);
	bool executeSql(QString sql, QSqlDatabase& db = QSqlDatabase::database());

	void refresh();

	void writeSettings();
	void readSettings();
	QSettings& settings() { return settings_; }

	// helpers
	void addSingleCustomIssue(int seriesId, QString number);

	QSettings settings_;
	AddComics* addComicsDialog;
  Ui::MainWindow *ui;
	bool m_connected;
};

#endif // MAINWINDOW_H
