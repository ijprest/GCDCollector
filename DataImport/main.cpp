#include "stdafx.h"

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

bool doSql(const QString& sql)
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
	if(a.arguments().count() < 2)
		return 1;

	// Open the database
	if(!createConnection())
		return 1;

	// Open the SQL file
	QFile file(a.arguments().at(1));

	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		cerr << QObject::tr("Error reading file: %1\n").arg(file.fileName()).toLatin1().data();
		return 1;
	}
	cout << QObject::tr("Opened file: %1\n").arg(file.fileName()).toLatin1().data();

	int lineno = 0;
	while(!file.atEnd()) 
	{
		QString line = QString(file.readLine()).trimmed();
		if(line.length() > 0) 
		{
			if(!doSql(line))
				cout << "Line " << ++lineno << ": " << line.left(50).toLatin1().data() << endl;
			else
				cout << "Line " << ++lineno << "\r";
		}
	}

	return 0; //a.exec();
}
