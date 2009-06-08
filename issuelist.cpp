/*SDOC:**********************************************************************

	File:			issuelist.cpp

	Action:		Implementation of the IssueList widget.

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
#include "issuelist.h"

/////////////////////////////////////////////////////////////////////////////
// IssueList widget class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			IssueList::IssueList
						IssueList::~IssueList

	Action:		Constructor / Destructor

**********************************************************************:EDOC*/
IssueList::IssueList(QWidget* parent)
  : QTableView(parent),
    model(NULL)
{
}

IssueList::~IssueList()
{
  delete model;
}


/*SDOC:**********************************************************************

	Name:			IssueList::setSeries

	Action:		SLOT that filters the issue list by the indicated series id.

	Params:		seriesId - id of the series to filter by

**********************************************************************:EDOC*/
void IssueList::setSeries(int seriesId)
{
	// Construct a new data model
  if(model) { delete model; model = NULL; }
  model = new QSqlQueryModel;
  model->setQuery(QString("SELECT Id, Number, publication_date, Price, page_count, notes "
													"FROM issues WHERE series_id=%1 ORDER BY sort_code").arg(seriesId));
	model->setHeaderData(0, Qt::Horizontal, "id");
	model->setHeaderData(1, Qt::Horizontal, tr("Number"));
	model->setHeaderData(2, Qt::Horizontal, tr("Date"));
	model->setHeaderData(3, Qt::Horizontal, tr("Price"));
	model->setHeaderData(4, Qt::Horizontal, tr("Pages"));
	model->setHeaderData(5, Qt::Horizontal, tr("Notes"));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
	resizeColumnsToContents();
}

/* end of file */