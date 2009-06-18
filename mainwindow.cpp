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

/////////////////////////////////////////////////////////////////////////////
// MainWindow window class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			MainWindow::MainWindow
						MainWindow::~MainWindow

	Action:		Constructor / destructor

**********************************************************************:EDOC*/
MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow),
		addComicsDialog(0)
{
	// Initialize UI
  ui->setupUi(this);

	// File menu
	connect(ui->action_New, SIGNAL(triggered()), this, SLOT(newDatabase()));
	connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(openDatabase()));
	connect(ui->action_Close, SIGNAL(triggered()), this, SLOT(closeDatabase()));
	connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));
	// Edit menu
	connect(ui->action_AddComics, SIGNAL(triggered()), this, SLOT(addComics()));
	connect(ui->action_DuplicateComic, SIGNAL(triggered()), ui->issueList, SLOT(duplicate()));
	connect(ui->action_Cut, SIGNAL(triggered()), ui->issueList, SLOT(cut())); ui->action_Cut->setShortcuts(QKeySequence::Cut);
	connect(ui->action_Copy, SIGNAL(triggered()), ui->issueList, SLOT(copy())); ui->action_Copy->setShortcuts(QKeySequence::Copy);
	connect(ui->action_Paste, SIGNAL(triggered()), ui->issueList, SLOT(paste())); ui->action_Paste->setShortcuts(QKeySequence::Paste);
	connect(ui->action_Delete, SIGNAL(triggered()), ui->issueList, SLOT(del())); ui->action_Delete->setShortcuts(QKeySequence::Delete);
	// View menu
	connect(ui->action_ShowOwnedIssues, SIGNAL(toggled(bool)), ui->issueList, SLOT(setShowOwned(bool)));
	connect(ui->action_ShowWantedIssues, SIGNAL(toggled(bool)), ui->issueList, SLOT(setShowWanted(bool)));
	connect(ui->action_ShowSoldIssues, SIGNAL(toggled(bool)), ui->issueList, SLOT(setShowSold(bool)));
	connect(ui->action_ShowUntrackedIssues, SIGNAL(toggled(bool)), ui->issueList, SLOT(setShowUntracked(bool)));
	// Help Menu
	connect(ui->action_About, SIGNAL(triggered()), this, SLOT(about()));
	connect(ui->action_AboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	// Other
	connect(ui->issueList, SIGNAL(selectionChanged(int)), this, SLOT(setCoverId(int)));
	connect(ui->issueList, SIGNAL(selectionChanged(int)), ui->coverView, SLOT(setImageId(int)));
	connect(ui->coverView, SIGNAL(downloadProgress(int,int)), ui->coverProgress, SLOT(setRange(int,int)));
	connect(ui->coverView, SIGNAL(downloadInProgress(bool)), ui->coverProgress, SLOT(setVisible(bool)));
	ui->coverProgress->setVisible(false);

	// Set the series-list to only show series we own
	ui->comicTitles->setOnlyShowOwned(true);

	// Handle command-line arguments
	for(int i = 1; i < QCoreApplication::arguments().size(); ++i)
	{
		QFileInfo fileInfo(QCoreApplication::arguments().at(i));
		if(fileInfo.exists() && fileInfo.completeSuffix() == "comicdb")
		{
			connectDatabase(fileInfo.absoluteFilePath());
		}
	}

	// The window icon (used by the about box)
	QIcon icon(":/GCDCollector/Resources/short-box.png");
	setWindowIcon(icon);
}

MainWindow::~MainWindow()
{
  delete ui;
	delete addComicsDialog;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::createDatabase

	Action:		Creates the necessary tables in the user-database.

**********************************************************************:EDOC*/
bool MainWindow::createDatabase(const QString& filename)
{
  if(!connectDatabase(filename))
    return false;

	// document.comics
	{
		QSqlQuery createTable;
		createTable.prepare("CREATE TABLE document.comics ("
													"id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
													"issue_id INTEGER NOT NULL, "
													"condition VARCHAR(32), "
													"store VARCHAR(32), "
													"price DOUBLE, "
													"notes VARCHAR, "
													"owned TINYINT(1) NOT NULL DEFAULT ('false'), "
													"sold_price DOUBLE, "
													"user_id VARCHAR(32));");
		if( !createTable.exec() )
		{
			QMessageBox::critical(0, tr("Database Error"), createTable.lastError().text());
			return false;
		}
	}

	{
		QSqlQuery createIndex;
		createIndex.prepare("CREATE INDEX document.comics_issueid on document.comics (issue_id ASC)");
		if( !createIndex.exec() )
		{
			QMessageBox::critical(0, tr("Database Error"), createIndex.lastError().text());
			return false;
		}
	}

	// document.images
	{
		QSqlQuery createTable;
		createTable.prepare("CREATE TABLE document.images (id INTEGER PRIMARY KEY NOT NULL, data BLOB);");
		if( !createTable.exec() )
		{
			QMessageBox::critical(0, tr("Database Error"), createTable.lastError().text());
			return false;
		}
	}

  return true;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::connectDatabase

	Action:		Attach to a user-database.

**********************************************************************:EDOC*/
bool MainWindow::connectDatabase(const QString& filename)
{
	// We ATTACH the user-database to the master database (instead of just 
	// opening it normally) so that we can do JOINs between the two.
	QSqlQuery attach;
	attach.prepare(QString("ATTACH DATABASE '%1' AS document;").arg(filename));
	if( !attach.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), attach.lastError().text());
		return false;
	}
  setWindowTitle(tr("Comic Collector - %1").arg(filename));

	// re-filter the main window's series list
	ui->comicTitles->filterList(ui->filterEdit->text());
  return true;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::closeDatabase

	Action:		Detach from a user-database.

**********************************************************************:EDOC*/
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


/*SDOC:**********************************************************************

	Name:			MainWindow::newDatabase

	Action:		Handles the File/New command

**********************************************************************:EDOC*/
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


/*SDOC:**********************************************************************

	Name:			MainWindow::openDatabase

	Action:		Handles the File/Open command

**********************************************************************:EDOC*/
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


/*SDOC:**********************************************************************

	Name:			MainWindow::about

	Action:		Handles the Help/About command

**********************************************************************:EDOC*/
void MainWindow::about()
{
	QMessageBox::about(this, tr("About Comic Collector"), 
		tr(	"<h2>Comic Collector</h2>"
				"<p>Copyright &copy; 2009, Ian Prest<br/>All rights reserved.</p>"
				"<p>This software is licensed under the terms of a modified BSD license; "
				"please see <a href='http://wiki.github.com/ijprest/GCDCollector/license'>"
				"http://wiki.github.com/ijprest/GCDCollector/license</a> for details.</p>"
				"<p>All comic data is copyright &copy; 1994-2009, GCD and GCD contributors; "
				"please see <a href='http://www.comics.org'>http://www.comics.org</a> for details.</p>"
				));
}


/*SDOC:**********************************************************************

	Name:			MainWindow::addComics

	Action:		Handles the Edit/Add Comics command

**********************************************************************:EDOC*/
void MainWindow::addComics()
{
	// The "add comics" dialog is modeless; if it hasn't been created 
	// yet, do so.
	if(!addComicsDialog)
	{
		addComicsDialog = new AddComics(this);
		connect(addComicsDialog, SIGNAL(addItems(QList<int>)), this, SLOT(addItems(QList<int>)));
	}
	// Show the dialog
	addComicsDialog->show();
	addComicsDialog->activateWindow();
}


/*SDOC:**********************************************************************

	Name:			MainWindow::addItems

	Action:		SLOT called by the "add items" dialog when the user wants to
						add comics to his collection.

**********************************************************************:EDOC*/
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

void MainWindow::setCoverId(int id)
{
	QSqlQuery query;
	query.prepare("SELECT issues.series_id, issues.sort_code, covers.has_image, covers.has_small, "
								"covers.has_medium, covers.has_large FROM issues INNER JOIN covers ON issues.id=covers.issue_id "
								"WHERE issues.id=?");
	query.addBindValue(id);
	query.exec();
	if(query.next()) // only expecting one row
	{
		/*
		href = QString("http://www.comics.org/graphics/covers/%1/400/%1_4_%2.jpg")
											.arg(query.value(0).toInt())			// series_id
											.arg(query.value(1).toString());	// sort_code
		*/

		int zoom = 0;
		if(query.value(5).toBool()) // large image
			zoom = 4;
		else if(query.value(4).toBool()) // medium image
			zoom = 2;
		else if(query.value(3).toBool()) // small image
			zoom = 1;

		if(zoom > 0)
		{
			QString href = QString("http://www.comics.org/coverview.lasso?id=%1&zoom=%2").arg(id).arg(zoom);
			ui->coverLink->setText(QString("<a href='%1'>See Cover Online</a>").arg(href));
			return;
		}
	}

	// cover wasn't available
	ui->coverLink->setText(tr("Drop a cover image here"));
}

/* end of file */