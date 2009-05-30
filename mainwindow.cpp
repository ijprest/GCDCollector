#include "stdafx.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

bool MainWindow::createDatabase(const QString& filename)
{
  if(!connectDatabase(filename))
    return false;

  QSqlDatabase db = QSqlDatabase::database("document");
  QSqlQuery("CREATE TABLE issues (id INTEGER PRIMARY KEY NOT NULL, issue_id INTEGER NOT NULL);", db).exec();
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

void MainWindow::on_action_New_triggered()
{
  // Get a new filename
  QString filename = QFileDialog::getSaveFileName(this, tr("New Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    on_action_Close_triggered();

    // Delete any existing file
    QFile::remove(filename);

    // Create a new database
    createDatabase(filename);
  }
}

void MainWindow::on_action_Open_triggered()
{
  // Get a new filename
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Database..."), ".", tr("Database files (*.comicdb);;All files (*.*)"));
  if(!filename.isEmpty())
  {
    // Close current database if one is open
    on_action_Close_triggered();

    // Open the new database
    connectDatabase(filename);
  }
}

void MainWindow::on_action_Close_triggered()
{
  QSqlDatabase::removeDatabase("document");
  setWindowTitle(tr("Comic Collector"));
}
