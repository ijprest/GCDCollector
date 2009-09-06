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

struct QueryHelper
{
	QSqlQuery query_;
	QueryHelper(const QString& sql) { query_.prepare(sql); }
	QueryHelper& arg(const QVariant& val) { query_.addBindValue(val); return *this; }
	operator QVariant() { return exec(); }
	QVariant exec() { if(!query_.exec() || !query_.next()) return QVariant(); return query_.value(0); }
};


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
		settings_("GCDCollector", "1.0"),
		addComicsDialog(0),
		m_connected(false)
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
	connect(this, SIGNAL(connected(bool)), ui->action_AddComics, SLOT(setEnabled(bool)));
	connect(ui->action_AddCustomSeries, SIGNAL(triggered()), this, SLOT(addCustomSeries()));
	connect(this, SIGNAL(connected(bool)), ui->action_AddCustomSeries, SLOT(setEnabled(bool)));
	connect(ui->action_AddCustomIssue, SIGNAL(triggered()), this, SLOT(addCustomIssue()));
	connect(this, SIGNAL(connected(bool)), ui->action_AddCustomIssue, SLOT(setEnabled(bool)));
	connect(ui->action_DuplicateComic, SIGNAL(triggered()), ui->issueList, SLOT(duplicate()));
	connect(this, SIGNAL(connected(bool)), ui->action_DuplicateComic, SLOT(setEnabled(bool)));
	connect(ui->action_Cut, SIGNAL(triggered()), ui->issueList, SLOT(cut())); ui->action_Cut->setShortcuts(QKeySequence::Cut);
	connect(this, SIGNAL(connected(bool)), ui->action_Cut, SLOT(setEnabled(bool)));
	connect(ui->action_Copy, SIGNAL(triggered()), ui->issueList, SLOT(copy())); ui->action_Copy->setShortcuts(QKeySequence::Copy);
	connect(this, SIGNAL(connected(bool)), ui->action_Copy, SLOT(setEnabled(bool)));
	connect(ui->action_Paste, SIGNAL(triggered()), ui->issueList, SLOT(paste())); ui->action_Paste->setShortcuts(QKeySequence::Paste);
	connect(this, SIGNAL(connected(bool)), ui->action_Paste, SLOT(setEnabled(bool)));
	connect(ui->action_Delete, SIGNAL(triggered()), ui->issueList, SLOT(del())); ui->action_Delete->setShortcuts(QKeySequence::Delete);
	connect(this, SIGNAL(connected(bool)), ui->action_Delete, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(connected(bool)), ui->action_Undo, SLOT(setEnabled(bool)));
	connect(this, SIGNAL(connected(bool)), ui->action_Redo, SLOT(setEnabled(bool)));
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

	// Restore UI state
	readSettings();

	// Handle command-line arguments
	for(int i = 1; i < QCoreApplication::arguments().size(); ++i)
	{
		QFileInfo fileInfo(QCoreApplication::arguments().at(i));
		if(fileInfo.exists() && fileInfo.completeSuffix() == "comicdb")
		{
			connectDatabase(fileInfo.absoluteFilePath());
		}
	}

	// Make sure UI state is consistent
	connected(m_connected);

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

	Name:			MainWindow::executeSql

	Action:		Prepare & execute a simple SQL statement

**********************************************************************:EDOC*/
bool MainWindow::executeSql(QString sql, QSqlDatabase& db)
{
	QSqlQuery query(db);
	query.prepare(sql);
	if( !query.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), query.lastError().text());
		return false;
	}
	return true;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::createDatabase

	Action:		Creates the necessary tables in the user-database.

**********************************************************************:EDOC*/
bool MainWindow::createDatabase(const QString& filename)
{
	// Create the database
	{
		QSqlDatabase newDb = QSqlDatabase::addDatabase("QSQLITE", "createDatabase");
		newDb.setDatabaseName(filename);
		if (!newDb.open()) 
		{
			QMessageBox::critical(0, QObject::tr("Database Error"), newDb.lastError().text());
			return false;
		}
		if(!executeSql("PRAGMA encoding = \"UTF-8\";", newDb))
			return false;

		// comics
		if(!executeSql("CREATE TABLE comics (id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, issue_id INTEGER NOT NULL, condition VARCHAR(32), store VARCHAR(32), price DOUBLE, notes VARCHAR, owned TINYINT(1) NOT NULL DEFAULT ('false'), sold_price DOUBLE, user_id VARCHAR(32));", newDb))
			return false;
		if(!executeSql("CREATE INDEX comics_issueid on comics (issue_id ASC)", newDb))
			return false;
		// images
		if(!executeSql("CREATE TABLE images (id INTEGER PRIMARY KEY NOT NULL, data BLOB);", newDb))
			return false;
		// custom_issues / custom_series
		if(!executeSql("CREATE TABLE custom_issues (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, number INTEGER, series_id INTEGER, sort_code INTEGER, publication_date VARCHAR);", newDb))
			return false;
		if(!executeSql("CREATE TABLE custom_series (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, name VARCHAR);", newDb))
			return false;
	}
	QSqlDatabase::removeDatabase("createDatabase");

  return connectDatabase(filename);
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

	// refresh the main window's series list
	refresh();
	connected(m_connected = true);
  return true;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::closeDatabase

	Action:		Detach from a user-database.

**********************************************************************:EDOC*/
void MainWindow::closeDatabase()
{
	if(!m_connected)
		return;

	QSqlQuery detach;
	detach.prepare("DETACH DATABASE document;");
	if( !detach.exec() )
	{
		QMessageBox::critical(0, tr("Database Error"), detach.lastError().text());
		return;
	}
	setWindowTitle(tr("Comic Collector"));
	connected(m_connected = false);

	// Clear the filter text and refresh the UI
	ui->filterEdit->setText("");
	// Clear the cover view
	ui->coverLink->setText("");
	ui->coverView->setImageId(0);
	ui->coverProgress->setVisible(false);
	// Clear the series detail view
	ui->groupBox->setSeries(0);
	// Refresh the rest of the UI
	refresh();
}


/*SDOC:**********************************************************************

	Name:			MainWindow::newDatabase

	Action:		Handles the File/New command

**********************************************************************:EDOC*/
bool MainWindow::newDatabase()
{
  // Get a new filename
  QString filename = QFileDialog::getSaveFileName(this, tr("New Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    closeDatabase();

    // Delete any existing file
		if(QFile::exists(filename) && !QFile::remove(filename))
		{
			QMessageBox::critical(this, tr("Database Error"), tr("Could not overwrite existing file."));
			return false;
		}

    // Create a new database
		if(!createDatabase(filename))
		{
			closeDatabase();
			return false;
		}
		return true;
  }
	return false;
}


/*SDOC:**********************************************************************

	Name:			MainWindow::openDatabase

	Action:		Handles the File/Open command

**********************************************************************:EDOC*/
bool MainWindow::openDatabase()
{
  // Get a new filename
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    closeDatabase();

    // Open the new database
    return connectDatabase(filename);
  }
	return false;
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
				"please see <a href='http://ijprest.github.com/GCDCollector/license.html'>"
				"http://ijprest.github.com/GCDCollector/license.html</a> for details.</p>"
				"<p>All comic data is copyright &copy; 1994-2009, GCD and GCD contributors; "
				"please see <a href='http://www.comics.org'>http://www.comics.org</a> for details.</p>"
				));
}


/*SDOC:**********************************************************************

	Name:			MainWindow::closeEvent

	Action:		Called when the user closes the window; will save user 
						preferences.

**********************************************************************:EDOC*/
void MainWindow::closeEvent(QCloseEvent* event)
{
	writeSettings();
	event->accept();
}


/*SDOC:**********************************************************************

	Name:			MainWindow::writeSettings
						MainWindow::readSettings

	Action:		Save/restore user preferences.

**********************************************************************:EDOC*/
void MainWindow::writeSettings()
{
	// Save window position
	settings().setValue("geometry", saveGeometry());
	// Save docker positions
	settings().setValue("dockerState", saveState());

	// Sub-dialogs
	if(addComicsDialog) addComicsDialog->writeSettings(settings());

	// Save show/hide flags
	settings().setValue("showOwned", ui->action_ShowOwnedIssues->isChecked());
	settings().setValue("showWanted", ui->action_ShowWantedIssues->isChecked());
	settings().setValue("showSold", ui->action_ShowSoldIssues->isChecked());
	settings().setValue("showUntracked", ui->action_ShowUntrackedIssues->isChecked());
}

void MainWindow::readSettings()
{
	// Restore window position
	restoreGeometry(settings().value("geometry").toByteArray());
	// Restore docker positions
	restoreState(settings().value("dockerState").toByteArray());

	// Restore show/hide flags
	ui->action_ShowOwnedIssues->setChecked(settings().value("showOwned", true).toBool());
	ui->action_ShowWantedIssues->setChecked(settings().value("showWanted", true).toBool());
	ui->action_ShowSoldIssues->setChecked(settings().value("showSold", true).toBool());
	ui->action_ShowUntrackedIssues->setChecked(settings().value("showUntracked", false).toBool());
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
		addComicsDialog->readSettings(settings());
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
			if(!executeSql(QString("INSERT INTO document.comics(issue_id) VALUES (%1);").arg(*i)))
			{
				db.rollback();
				return;
			}
		}
		db.commit();

		// refresh the main window's series list
		refresh();
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
		int zoom = 0;
		if(query.value(5).toBool()) // large image
			zoom = 4;
		else if(query.value(4).toBool()) // medium image
			zoom = 2;
		else if(query.value(3).toBool()) // small image
			zoom = 1;
		if(zoom > 0)
		{
			ui->coverLink->setText(QString("<a href='http://www.comics.org/coverview.lasso?id=%1&zoom=%2'>%3</a>").arg(id).arg(zoom).arg(tr("See Cover Online")));
			return;
		}
	}

	// cover wasn't available
	ui->coverLink->setText(tr("No online cover available"));
}


/*SDOC:**********************************************************************

	Name:			MainWindow::addCustomSeries (SLOT)
						MainWindow::addCustomIssue (SLOT)

	Action:		Prompts the user for a name and inserts a custom series/issue

**********************************************************************:EDOC*/
void MainWindow::addCustomSeries()
{
	QString name = QInputDialog::getText(this, tr("Add Custom Series"), tr("Enter name for custom series:"));
	if(!name.isEmpty())
	{
		QSqlQuery query;
		query.prepare("INSERT INTO document.custom_series(name) VALUES (?);");
		query.addBindValue(name);
		if(!query.exec())
		{
			QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
			return;
		}

		// Re-filter the series list to (hopefully) show the new series.
		ui->comicTitles->filterList(ui->filterEdit->text());
	}
}

void MainWindow::addCustomIssue()
{
	int seriesId = ui->comicTitles->selectedSeries();
	if(seriesId != 0)
	{
		// Try the lookup in the main issue list & custom issue list
		int currentMaxValue = std::max(
			QueryHelper("SELECT MAX(CAST(number AS INTEGER)) FROM issues WHERE series_id=?").arg(seriesId).exec().toInt(),
			QueryHelper("SELECT MAX(CAST(number AS INTEGER)) FROM document.custom_issues WHERE series_id=?").arg(seriesId).exec().toInt());

		QString number = QInputDialog::getText(this, tr("Add Custom Issue"), tr("Enter number for custom issue:"), QLineEdit::Normal, QString("%1").arg(currentMaxValue + 1));
		if(!number.isEmpty())
		{
			QSqlQuery query;
			query.prepare("INSERT INTO document.custom_issues(series_id,number) VALUES (?,?);");
			query.addBindValue(seriesId);
			query.addBindValue(number);
			if(!query.exec())
			{
				QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
				return;
			}

			// Refresh view
			ui->issueList->refresh();
		}
	}
}


/*SDOC:**********************************************************************

	Name:			MainWindow::refresh

	Action:		Helper function to refresh the main UI

**********************************************************************:EDOC*/
void MainWindow::refresh()
{
	// Refresh the filter-list
	ui->comicTitles->filterList(ui->filterEdit->text());
	// Refresh the issue-list
	ui->issueList->refresh();
}

/* end of file */