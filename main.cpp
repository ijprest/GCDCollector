#include "stdafx.h"
#include "mainwindow.h"

bool createConnection()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("C:\\Workspaces\\ComicCollector\\comix.db"); // TODO: hardcoded location
  if (!db.open()) 
	{
    QMessageBox::critical(0, QObject::tr("Database Error"), db.lastError().text());
    return false;
  }
  return true;
}


int main(int argc, char *argv[])
{
  // Run the GUI
  QApplication app(argc, argv);
  if (!createConnection())
    return 1;
  MainWindow w;
  w.show();
  return app.exec();
}
