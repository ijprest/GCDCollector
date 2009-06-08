/*SDOC:**********************************************************************

	File:			comiclist.cpp

	Action:		Implementation of the ComicList object.

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
#include "comiclist.h"

/////////////////////////////////////////////////////////////////////////////
// ComicDataModel class
/////////////////////////////////////////////////////////////////////////////
class ComicDataModel : public QSqlQueryModel
{
public:
	enum columnIds { colId, colNumber, colOwned, colDate, colCondition, colPrice, colStore, colUserId, colNotes };
	static QString columnNameDb[];
	static QString columnNameUi[];

	ComicDataModel(int seriesId, QObject *parent = 0);
	Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
};

// These must be kept synchronized with the ComicDataModel::columnIds enum
QString ComicDataModel::columnNameDb[] = 
{ 
	"document.comics.id",					// colId
	"issues.number",							// colNumber
	"document.comics.owned",			// colOwned
	"issues.publication_date",		// colDate
	"document.comics.condition",	// colCondition
	"document.comics.price",			// colPrice
	"document.comics.store",			// colStore
	"document.comics.user_id",		// colUserId
	"document.comics.notes",			// colNotes
};
QString ComicDataModel::columnNameUi[] = 
{
	"Id",													// colId
	QObject::tr("Number"),				// colNumber
	"",														// colOwned
	QObject::tr("Date"),					// colDate
	QObject::tr("Condition"),			// colCondition
	QObject::tr("Price Paid"),		// colPrice
	QObject::tr("Store"),					// colStore
	QObject::tr("Id"),						// colUserId
	QObject::tr("Notes"),					// colNotes
};


/*SDOC:**********************************************************************

	Name:			ComicDataModel::ComicDataModel

	Action:		Constructs the data model & populates it with initial data

**********************************************************************:EDOC*/
ComicDataModel::ComicDataModel(int seriesId, QObject *parent) 
	: QSqlQueryModel(parent)
{
	// Build a list of column names
	QStringList dbNames;
	for(int i = 0; i < _countof(columnNameDb); ++i)
		dbNames.push_back(columnNameDb[i]);

	// Prepare & execute the query
	QString sql = QString("SELECT %1 "
												"FROM document.comics "
												"INNER JOIN issues ON issues.id = document.comics.issue_id "
												"WHERE issues.series_id = %2 "
												"ORDER BY issues.sort_code;").arg(dbNames.join(",")).arg(seriesId);
	setQuery(sql);

	// Set up the UI names for each column
	for(int i = 0; i < _countof(columnNameUi); ++i)
		setHeaderData(i, Qt::Horizontal, columnNameUi[i]);
}


/*SDOC:**********************************************************************

	Name:			ComicDataModel::flags

	Action:		Return editability flags for each column

**********************************************************************:EDOC*/
Qt::ItemFlags ComicDataModel::flags(const QModelIndex &index) const
{
	switch(index.column())
	{
	case colOwned:
		return QSqlQueryModel::flags(index) | Qt::ItemIsUserCheckable;
	case colCondition:
	case colPrice:
	case colStore:
	case colUserId:
	case colNotes:
		return QSqlQueryModel::flags(index) | Qt::ItemIsEditable;
	default:
		return QSqlQueryModel::flags(index);
	}
}


/*SDOC:**********************************************************************

	Name:			ComicDataModel::data

	Action:		Retrieve the data for each cell

**********************************************************************:EDOC*/
QVariant ComicDataModel::data(const QModelIndex &index, int role) const
{
	switch(index.column())
	{
	// For the "owned" column, we show a checbox instead of the string value
	case colOwned: 
		switch(role)
		{
		case Qt::CheckStateRole:
			return QSqlQueryModel::data(index).toBool() ? Qt::Checked : Qt::Unchecked;
		default:
			return QString();
		}
		break;

	// For the "price" column, we show the DOUBLE value as currency
	case colPrice: 
		switch(role)
		{
			case Qt::TextAlignmentRole:
				return Qt::AlignRight;
			case Qt::DisplayRole:
			{
				QVariant value = QSqlQueryModel::data(index, role);
				if(!value.isNull())
					return QString("$%1").arg(value.toDouble(),0,'f',2);
			}
		}
		break;
	}

	// Use the default behaviour
	return QSqlQueryModel::data(index, role);
}


/*SDOC:**********************************************************************

	Name:			ComicDataModel::setData

	Action:		Called in response to a user-edit; sets the new value for 
						a cell.

**********************************************************************:EDOC*/
bool ComicDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	// Get the database identifier for the row
	int rowId = QSqlQueryModel::data(QSqlQueryModel::index(index.row(), 0)).toInt();

	// Prepare the UPDATE query
	QSqlQuery updateQuery;
	QString tableName = columnNameDb[index.column()].section('.',0,-2);
	QString columnName = columnNameDb[index.column()].section('.',-1,-1);
	updateQuery.prepare(QString("UPDATE %1 SET %2=? WHERE id=?").arg(tableName, columnName));

	// Add the user-edited value
	switch(index.column())
	{
		case colOwned:
			if(role != Qt::CheckStateRole)
				return false;
			updateQuery.addBindValue(value == Qt::Checked);
			break;
		case colPrice:
			updateQuery.addBindValue((value.toString().length()==0) ? QVariant(QVariant::Double) : value.toDouble());
			break;
		case colCondition:
		case colStore:
		case colUserId:
		case colNotes:
			updateQuery.addBindValue((value.toString().length()==0) ? QVariant(QVariant::String) : value.toString());
			break;
		default:
			return false;
	}

	// Finalize & execute
	updateQuery.addBindValue(rowId);
	if(!updateQuery.exec())
	{
    QMessageBox::critical(0, QObject::tr("Database Error"), updateQuery.lastError().text());
		return false;
	}

	// Re-run the main query & tell any views that our 
	// data has changed.
	int pos = query().at();
	query().exec();
	query().seek(pos);
	dataChanged(index,index);
	return true;
}


/*SDOC:**********************************************************************

	Name:			ComicDataModel::headerData

	Action:		Returns the text to display in the header

**********************************************************************:EDOC*/
QVariant ComicDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// Use the comic issue number as the vertical header text
	if(orientation == Qt::Vertical && role == Qt::DisplayRole)
		return data(index(section,colNumber), role);
	return QSqlQueryModel::headerData(section, orientation, role);
}


/////////////////////////////////////////////////////////////////////////////
// ComicList class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			ComicList::ComicList
						ComicList::~ComicList

	Action:		Constructor / Destructor

**********************************************************************:EDOC*/
ComicList::ComicList(QWidget* parent)
  : QTableView(parent),
    model(0)
{
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
}

ComicList::~ComicList()
{
  delete model;
}


/*SDOC:**********************************************************************

	Name:			ComicList::setSeries (SLOT)

	Action:		Filters the list of comics by the selected series

	Params:		seriesId - the id of the series to filter by

**********************************************************************:EDOC*/
void ComicList::setSeries(int seriesId)
{
  if(model) { delete model; model = NULL; }
  model = new ComicDataModel(seriesId);
  setModel(model);
	setColumnHidden(ComicDataModel::colId, true);			// Hide the "id" column
	setColumnHidden(ComicDataModel::colNumber, true); // Hide the "number" column
	resizeColumnsToContents();
}

/* end of file */