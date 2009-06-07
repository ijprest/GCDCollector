#include "stdafx.h"
#include "comiclist.h"

ComicDataModel::ColumnDef ComicDataModel::columns[] = 
{
	{ "document.comics.id", "Id", false, QVariant::Invalid },
	{ "issues.number", tr("Number"), false, QVariant::Invalid },
	{ "issues.publication_date", tr("Date"), false, QVariant::Invalid },
	{ "document.comics.condition", tr("Condition"), true, QVariant::String },
	{ "document.comics.price", tr("Price Paid"), true, QVariant::Double },
	{ "document.comics.store", tr("Store"), true, QVariant::String },
	{ "document.comics.notes", tr("Notes"), true, QVariant::String },
};

ComicDataModel::ComicDataModel(int seriesId, QObject *parent) 
	: QSqlQueryModel(parent),
		seriesId(seriesId)
{
	refresh();
}

Qt::ItemFlags ComicDataModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QSqlQueryModel::flags(index);
	if(columns[index.column()].isEditable)
		flags |= Qt::ItemIsEditable;
	return flags;
}

bool ComicDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!columns[index.column()].isEditable)
		return false;

	int rowId = QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 0)).toInt();
	QVariant convertedValue = (value.toString().length()==0) ? QVariant(columns[index.column()].type) : value;
	convertedValue.convert(columns[index.column()].type);

	QString tableName = columns[index.column()].dbName.section('.',0,-2);
	QString columnName = columns[index.column()].dbName.section('.',-1,-1);

	QSqlQuery query;
	query.prepare(QString("UPDATE %1 SET %2=? WHERE id=?").arg(tableName, columnName));
	query.addBindValue(convertedValue);
	query.addBindValue(rowId);
	if(!query.exec())
	{
    QMessageBox::critical(0, QObject::tr("Database Error"), query.lastError().text());
		return false;
	}
	refresh();
	return true;
}

QVariant ComicDataModel::data(const QModelIndex &index, int role) const
{
	QVariant value = QSqlQueryModel::data(index, role);
	if(value.isValid() && role == Qt::DisplayRole)
	{
		if(index.column() == 4 && !value.isNull())  // TODO: hardcoded number
		{
			return QString("$%1").arg(value.toDouble(),0,'f',2);
		}
	}
	return value;
}

void ComicDataModel::refresh()
{
	QStringList dbNames;
	for(int i = 0; i < sizeof(columns)/sizeof(columns[0]); ++i)
		dbNames.push_back(columns[i].dbName);

	QString sql = QString("SELECT %1 "
												"FROM document.comics "
												"INNER JOIN issues ON issues.id = document.comics.issue_id "
												"WHERE issues.series_id = %2 "
												"ORDER BY issues.sort_code;").arg(dbNames.join(",")).arg(seriesId);
	setQuery(sql);

	for(int i = 0; i < sizeof(columns)/sizeof(columns[0]); ++i)
		setHeaderData(i, Qt::Horizontal, columns[i].uiName);
}

ComicList::ComicList(QWidget* parent)
  : QTableView(parent),
    model(0)
{
}

ComicList::~ComicList()
{
  delete model;
}

void ComicList::setSeries(int seriesId)
{
  if(model) { delete model; model = NULL; }
  model = new ComicDataModel(seriesId);
  setModel(model);
	setColumnHidden(0, true); // Hide the "id" column
	resizeColumnsToContents();
}
