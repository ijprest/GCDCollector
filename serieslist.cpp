/*SDOC:**********************************************************************

	File:			serieslist.cpp

	Action:		Implementation of the SeriesList widget.

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
#include "serieslist.h"

/////////////////////////////////////////////////////////////////////////////
// SeriesList widget class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			SeriesList::SeriesList
						SeriesList::~SeriesList

	Action:		Constructor / Destructor

**********************************************************************:EDOC*/
SeriesList::SeriesList(QWidget* parent)
  : QTableView(parent),
		onlyShowOwned(false),
    model(NULL)
{
}

SeriesList::~SeriesList()
{
  delete model;
}


/*SDOC:**********************************************************************

	Name:			SeriesList::filterList

	Action:		SLOT called to filter the list by a given string

	Params:		filter - string by which to filter the list

**********************************************************************:EDOC*/
void SeriesList::filterList(const QString& filter)
{
  if(model) { delete model; model = NULL; }
  model = new QSqlQueryModel;

	if(onlyShowOwned)
	{
		model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM series "
														"WHERE name LIKE '%%1%' AND id IN ("
															"SELECT DISTINCT series_id FROM issues WHERE issues.id IN (SELECT issue_id FROM document.comics)"
														")"
														"ORDER BY Name").arg(filter));
	}
	else
	{
		model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM series "
														"WHERE name LIKE '%%1%' "
														"ORDER BY Name").arg(filter));
	}
  model->setHeaderData(0,Qt::Horizontal,"Id");
  model->setHeaderData(1,Qt::Horizontal,tr("Name"));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
  connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChange(QModelIndex)));
}


/*SDOC:**********************************************************************

	Name:			SeriesList::selectionChange

	Action:		SLOT called to when the user's selection changes; emits a 
						"seriesSelected" signal.

**********************************************************************:EDOC*/
void SeriesList::selectionChange(const QModelIndex& index)
{
  // Retrieve the series ID (column 0 in the selected row)
  int seriesId = index.sibling(index.row(),0).data().toInt();
  seriesSelected(seriesId);
}

/* end of file */