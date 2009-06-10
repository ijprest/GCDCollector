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
	enum columnIds { colId, colIssueId, colNumber, colOwned, colDate, colCondition, colPrice, colStore, colUserId, colNotes };
	static QString columnNameDb[];
	static QString columnNameUi[];

	ComicDataModel(int seriesId, bool showOwned, bool showWanted, bool showSold, bool showUntracked, QObject *parent = 0);
	Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

	bool setDataInternal(const QModelIndex &index, const QVariant &value, int role);
	void refreshView(const QItemSelectionRange range);
};

// These must be kept synchronized with the ComicDataModel::columnIds enum
QString ComicDataModel::columnNameDb[] = 
{ 
	"document.comics.id",					// colId
	"document.comics.issue_id",		// colIssueId
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
	"Issue",											// colIssueId
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
ComicDataModel::ComicDataModel(int seriesId, bool showOwned, bool showWanted, bool showSold, bool showUntracked, QObject *parent) 
	: QSqlQueryModel(parent)
{
	// Build a list of column names
	QStringList dbNames;
	for(int i = 0; i < _countof(columnNameDb); ++i)
		dbNames.push_back(columnNameDb[i]);

	// Prepare & execute the query
	QString conditions;
	if(      showOwned &&  showWanted &&  showSold) conditions = "";
	else if( showOwned &&  showWanted && !showSold) conditions = "AND (document.comics.owned = 'true' OR document.comics.sold_price IS NULL)";
	else if( showOwned && !showWanted &&  showSold) conditions = "AND (document.comics.owned = 'true' OR document.comics.sold_price IS NOT NULL)";
	else if( showOwned && !showWanted && !showSold) conditions = "AND document.comics.owned = 'true'";
	else if(!showOwned &&  showWanted &&  showSold) conditions = "AND document.comics.owned = 'false'";
	else if(!showOwned &&  showWanted && !showSold) conditions = "AND document.comics.owned = 'false' AND document.comics.sold_price IS NULL";
	else if(!showOwned && !showWanted &&  showSold) conditions = "AND document.comics.owned = 'false' AND document.comics.sold_price IS NOT NULL";
	else if(!showOwned && !showWanted && !showSold) conditions = "AND document.comics.id = -1";
	QString sql = QString("SELECT %1 "
												"FROM document.comics "
												"INNER JOIN issues ON issues.id = document.comics.issue_id "
												"WHERE issues.series_id = %2 %3 "
												"ORDER BY issues.sort_code;").arg(dbNames.join(",")).arg(seriesId).arg(conditions);
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
		case Qt::DisplayRole:
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
						ComicDataModel::setDataInternal

	Action:		Called in response to a user-edit; sets the new value for 
						a cell.

**********************************************************************:EDOC*/
bool ComicDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(setDataInternal(index, value, role))
	{
		// Re-run the main query & tell any views that our 
		// data has changed.
		int pos = query().at();
		query().exec();
		query().seek(pos);
		dataChanged(index,index);
		return true;
	}
	return false;
}

bool ComicDataModel::setDataInternal(const QModelIndex &index, const QVariant &value, int role)
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
			if(role != Qt::CheckStateRole && role != Qt::EditRole)
				return false;
			updateQuery.addBindValue(value == Qt::Checked || value == "true");
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
	return true;
}

void ComicDataModel::refreshView(const QItemSelectionRange range)
{
	// Re-run the main query & tell any views that our 
	// data has changed.
	int pos = query().at();
	query().finish();
	query().exec();
	query().seek(pos);
	dataChanged(range.topLeft(), range.bottomRight());
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
    _model(0),
		seriesId(-1),
		showOwned(true),
		showWanted(true),
		showSold(true),
		showUntracked(false)
{
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
}

ComicList::~ComicList()
{
  delete _model;
}


/*SDOC:**********************************************************************

	Name:			ComicList::setModel (SLOT)

	Action:		Override the base-class' setModel to perform some additional 
						work.

**********************************************************************:EDOC*/
void ComicList::setModel(QAbstractItemModel* newModel)
{
	if(_model) { delete _model; }
	_model = newModel;

	QTableView::setModel(_model);
	connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChange(QModelIndex)));
	setColumnHidden(ComicDataModel::colId, true);			// Hide the "id" column
	setColumnHidden(ComicDataModel::colIssueId, true);// Hide the "issue id" column
	setColumnHidden(ComicDataModel::colNumber, true); // Hide the "number" column
	resizeColumnsToContents();
}


/*SDOC:**********************************************************************

	Name:			ComicList::setSeries (SLOT)

	Action:		Filters the list of comics by the selected series

	Params:		seriesId - the id of the series to filter by

**********************************************************************:EDOC*/
void ComicList::setSeries(int _seriesId)
{
  setModel(new ComicDataModel(seriesId = _seriesId, showOwned, showWanted, showSold, showUntracked, this));
}


/*SDOC:**********************************************************************

	Name:			ComicList::setShowOwned (SLOT)
						ComicList::setShowWanted (SLOT)
						ComicList::setShowSold (SLOT)
						ComicList::setShowUntracked (SLOT)

	Action:		Toggles filtering is the comic list by various criteria

**********************************************************************:EDOC*/
void ComicList::setShowOwned(bool show)
{
  setModel(new ComicDataModel(seriesId, showOwned = show, showWanted, showSold, showUntracked, this));
}

void ComicList::setShowWanted(bool show)
{
  setModel(new ComicDataModel(seriesId, showOwned, showWanted = show, showSold, showUntracked, this));
}

void ComicList::setShowSold(bool show)
{
  setModel(new ComicDataModel(seriesId, showOwned, showWanted, showSold = show, showUntracked, this));
}

void ComicList::setShowUntracked(bool show)
{
  setModel(new ComicDataModel(seriesId, showOwned, showWanted, showSold, showUntracked = show, this));
}


/*SDOC:**********************************************************************

	Name:			ComicList::cut (SLOT)
						ComicList::copy (SLOT)
						ComicList::paste (SLOT)

	Action:		Clipboard handling

**********************************************************************:EDOC*/
void ComicList::cut()
{
	copy();
	del();
}

void ComicList::copy()
{
	if(!selectionModel()->hasSelection())
		return;
  const QItemSelectionRange selection = selectionModel()->selection().first();

	// Copy tab-separated data to the clipboard
	QString str;
	for(int row = selection.top(); row <= selection.bottom(); ++row) 
	{
		if(row > selection.top()) str += "\n";
		for(int column = selection.left(); column <= selection.right(); ++column) 
		{
			if(column > selection.left()) str += "\t";
			str += model()->index(row, column).data(Qt::EditRole).toString();
		}
	}
	QApplication::clipboard()->setText(str);
}

void ComicList::paste()
{
	if(!selectionModel()->hasSelection())
		return;
  QItemSelectionRange selection = selectionModel()->selection().first();

	// Expect clipboard data as tab-separated values
	QString str = QApplication::clipboard()->text();
	QStringList rows = str.split('\n');
	if(rows.back().length() == 0) rows.pop_back();
	int numRows = rows.count();
	int numColumns = rows.first().count('\t') + 1;

	// We don't support the paste operation unless:
	if( selection.height() * selection.width() != 1 &&  // selection is 1x1 (might still be pasting n x m)
			numRows * numColumns != 1 &&										// paste is 1x1 (selection might still be n x m; data is duplicated)
			(selection.height() != numRows ||								// selection size == paste size
			 selection.width() != numColumns)) 
	{
		QMessageBox::critical(this, tr("Comic Collector"), 
			tr("The information cannot be pasted because the copy and paste areas aren't the same size."));
		return;
	}

	if(numRows == 1 && numColumns == 1)
	{
		// Only one piece of data that we're pasting multiple cells
		for(int row = selection.top(); row <= selection.bottom(); ++row) 
		{
			for(int column = selection.left(); column <= selection.right(); ++column) 
				((ComicDataModel*)_model)->setDataInternal(model()->index(row,column), str, Qt::EditRole);
		}
	}
	else 
	{
		selection = QItemSelectionRange(selection.topLeft(), selection.topLeft().sibling(selection.top()+numRows-1, selection.left()+numColumns-1));
		for(int row = selection.top(); row <= selection.bottom(); ++row) 
		{
			// Split the line of text
			QStringList rowData = rows[row - selection.top()].split('\t');
			// Paste into the appropriate cell
			for(int column = selection.left(); column <= selection.right(); ++column) 
				((ComicDataModel*)_model)->setDataInternal(model()->index(row,column), rowData[column-selection.left()], Qt::EditRole);
		}
	}
	((ComicDataModel*)_model)->refreshView(selection);
}

void ComicList::del()
{
	if(!selectionModel()->hasSelection())
		return;
  QItemSelectionRange selection = selectionModel()->selection().first();

	if(selection.left() == ComicDataModel::colId)
	{
		// The user has requested that we delete entire records (by selecting
		// entire rows using the vertical header).  Make sure this is what they
		// wanted:
		if( QMessageBox::question(this, tr("Confirm record delete"), 
			tr("Are you sure you want to delete the selected records?"), 
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes )
		{
			QStringList ids;
			for(int row = selection.top(); row <= selection.bottom(); ++row) 
				ids.push_back(model()->data(model()->index(row,ComicDataModel::colId), Qt::EditRole).toString());

			QString sql = QString("DELETE FROM document.comics WHERE document.comics.id IN (%1);").arg(ids.join(","));
			QSqlQuery query;
			query.prepare(sql);
			query.exec();

			// refresh by passing a brand-new model
			setModel(new ComicDataModel(seriesId, showOwned, showWanted, showSold, showUntracked, this));
		}
	}
	else
	{
		// Just delete the contents of all the cells in the selection
		for(int row = selection.top(); row <= selection.bottom(); ++row) 
		{
			for(int column = selection.left(); column <= selection.right(); ++column) 
				((ComicDataModel*)_model)->setDataInternal(model()->index(row,column), QVariant(), Qt::EditRole);
		}
		((ComicDataModel*)_model)->refreshView(selection);
	}
}

void ComicList::selectionChange(const QModelIndex& index)
{
	int rowId = model()->data(index.sibling(index.row(),ComicDataModel::colIssueId)).toInt();
	selectionChanged(rowId);
}

/* end of file */