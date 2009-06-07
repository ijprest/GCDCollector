/*SDOC:**********************************************************************

	File:			mainwindow.cpp

	Action:		Implementation of the MainWindow window.

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
#include "stdafx.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcomics.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow),
		addComicsDialog(0)
{
	// Initialize UI
  ui->setupUi(this);
	connect(ui->action_New, SIGNAL(triggered()), this, SLOT(newDatabase()));
	connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(openDatabase()));
	connect(ui->action_Close, SIGNAL(triggered()), this, SLOT(closeDatabase()));
	connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui->action_AddComics, SIGNAL(triggered()), this, SLOT(addComics()));
	connect(ui->action_AboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	connect(ui->action_About, SIGNAL(triggered()), this, SLOT(about()));

	// Handle command-line arguments
	for(int i = 1; i < QCoreApplication::arguments().size(); ++i)
	{
		QFileInfo fileInfo(QCoreApplication::arguments().at(i));
		if(fileInfo.exists() && fileInfo.completeSuffix() == "comicdb")
		{
			connectDatabase(fileInfo.absoluteFilePath());
		}
	}

	// Set the series-list to only show series we own
	ui->comicTitles->setOnlyShowOwned(true);
	ui->comicTitles->filterList("");

	// The window icon (used by the about box)
	QIcon icon(":/GCDCollector/Resources/short-box.png");
	setWindowIcon(icon);
}

MainWindow::~MainWindow()
{
  delete ui;
	delete addComicsDialog;
}

bool MainWindow::createDatabase(const QString& filename)
{
  if(!connectDatabase(filename))
    return false;

	QSqlQuery createTable;
	createTable.prepare("CREATE TABLE document.comics ("
												"id INTEGER PRIMARY KEY NOT NULL, "
												"issue_id INTEGER NOT NULL, "
												"condition VARCHAR(32), "
												"store VARCHAR(32), "
												"price DOUBLE, "
												"notes VARCHAR, "
												"owned tinyint(1), "
												"sold_price DOUBLE, "
												"user_id VARCHAR(32));");
	if( !createTable.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), createTable.lastError().text());
		return false;
	}
  return true;
}

bool MainWindow::connectDatabase(const QString& filename)
{
	QSqlQuery attach;
	attach.prepare(QString("ATTACH DATABASE '%1' AS document;").arg(filename));
	if( !attach.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), attach.lastError().text());
		return false;
	}
  setWindowTitle(tr("Comic Collector - %1").arg(filename));
  return true;
}

void MainWindow::newDatabase()
{
  // Get a new filename
  QString filename = QFileDialog::getSaveFileName(this, tr("New Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    closeDatabase();

    // Delete any existing file
    QFile::remove(filename);

    // Create a new database
    createDatabase(filename);
  }
}

void MainWindow::openDatabase()
{
  // Get a new filename
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    closeDatabase();

    // Open the new database
    connectDatabase(filename);
  }
}

void MainWindow::closeDatabase()
{
	QSqlQuery detach;
	detach.prepare("DETACH DATABASE document;");
	if( !detach.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), detach.lastError().text());
		return;
	}
  setWindowTitle(tr("Comic Collector"));
}

void MainWindow::addComics()
{
	if(!addComicsDialog)
	{
		addComicsDialog = new AddComics(this);
		connect(addComicsDialog, SIGNAL(addItems(QList<int>)), this, SLOT(addItems(QList<int>)));
	}
	addComicsDialog->show();
	addComicsDialog->activateWindow();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Comic Collector"), 
		tr(	"<h2>Comic Collector</h2>"
				"<p>Copyright &copy; 2009, Ian Prest<br/>All rights reserved.</p>"
				"<p>This software is licensed under the terms of a modified BSD license; "
				"please see <a href='http://wiki.github.com/ijprest/GCDCollector/license'>"
				"http://wiki.github.com/ijprest/GCDCollector/license</a> for details.</p>"
				));
}

void MainWindow::addItems(const QList<int>& items)
{
	QSqlDatabase db = QSqlDatabase::database();
	if(db.isValid() && db.transaction())
	{
		for(QList<int>::const_iterator i = items.begin(); i != items.end(); ++i)
		{
			QSqlQuery query(db);
			query.prepare(QString("INSERT INTO document.comics(issue_id) VALUES (%1);").arg(*i));
			if(!query.exec())
			{
				QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
				db.rollback();
				return;
			}
		}
		db.commit();

		// re-filter the main window's series list
		ui->comicTitles->filterList(ui->filterEdit->text());
	}
}