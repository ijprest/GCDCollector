#include "stdafx.h"
#include "sql.h"

bool createConnection()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("new.db");
  if (!db.open()) 
	{
		cerr << QObject::tr("Database Error: %1").arg(db.lastError().text()).toLatin1().data() << endl;
    return false;
  }
  return true;
}

bool runQuery(const QString& sql)
{
	QSqlQuery query;
	query.prepare(sql);
	if(!query.exec())
	{
		cerr << QObject::tr("Database Error: %1").arg(query.lastError().text()).toLatin1().data() << endl;
		return false;
	}
	return true;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// Open the database
	if(!createConnection())
		return 1;

	// Open the SQL file
	QFile file("..\\full-2.sql");
	//QFile file("C:\\Users\\Ian\\Desktop\\foo.txt");
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cerr << QObject::tr("Error reading file: %1\n").arg(file.fileName()).toLatin1().data();
		return 1;
	}
	cout << QObject::tr("Opened file: %1\n").arg(file.fileName()).toLatin1().data();

	Parser parser(file);
	if(!parser.parse())
	{
		cout << QString("%1(%2): %3").arg(file.fileName()).arg(parser.errorLineNumber()).arg(parser.errorMessage()).toLatin1().data() << endl;
	}

	return 0; //a.exec();
}
