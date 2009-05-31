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

  QSqlDatabase db = QSqlDatabase::database("document");
  QSqlQuery("CREATE TABLE issues (id INTEGER PRIMARY KEY NOT NULL, issue_id INTEGER NOT NULL, condition VARCHAR(32), store VARCHAR(32), price DOUBLE, notes VARCHAR);", db).exec();
  return true;
}

bool MainWindow::connectDatabase(const QString& filename)
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "document");
  db.setDatabaseName(filename);
  if(!db.open())
  {
    QMessageBox::critical(0, QObject::tr("Database Error"), db.lastError().text());
    return false;
  }
  setWindowTitle(tr("%1 - Comic Collector").arg(filename));
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
  QSqlDatabase::removeDatabase("document");
  setWindowTitle(tr("Comic Collector"));
}

void MainWindow::addComics()
{
	if(!addComicsDialog)
	{
		addComicsDialog = new AddComics(this);
	}
	addComicsDialog->show();
	addComicsDialog->activateWindow();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Comic Collector"), 
		tr(	"<h2>Comic Collector</h2>"
				"<p>Copyright &copy; 2009, Ian Prest<br/>All rights reserved.</p>"
				"<p>This software is licensed under the terms of a modified BSD license; please see <a href='http://wiki.github.com/ijprest/GCDCollector/license'>http://wiki.github.com/ijprest/GCDCollector/license</a> for details.</p>"
				));
}
