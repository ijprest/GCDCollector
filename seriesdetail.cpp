/*SDOC:**********************************************************************

	File:			seriesdetail.cpp

	Action:		Implementation of the SeriesDetail widget.

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
#include "seriesdetail.h"
#include "ui_seriesdetail.h"

/////////////////////////////////////////////////////////////////////////////
// SeriesDetail widget class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			SeriesDetail::SeriesDetail
						SeriesDetail::~SeriesDetail

	Action:		Constructor / Destructor

**********************************************************************:EDOC*/
SeriesDetail::SeriesDetail(QWidget *parent) :
  QGroupBox(parent),
  m_ui(new Ui::SeriesDetail)
{
  m_ui->setupUi(this);
}

SeriesDetail::~SeriesDetail()
{
  delete m_ui;
}


/*SDOC:**********************************************************************

	Name:			SeriesDetail::setSeries

	Action:		SLOT called to display the data of a given series

	Params:		seriesId - the id of the series to display

**********************************************************************:EDOC*/
void SeriesDetail::setSeries(int seriesId)
{
  QSqlQuery query;
  query.exec(QString("SELECT series.name, series.format, series.year_began, publishers.name "
										 "FROM series INNER JOIN publishers "
										 "WHERE publisher_id=publishers.id AND series.id = %1;").arg(seriesId));
  if(query.next()) // only expecting one row
  {
    QString label = QString("%1 (%2)").arg(query.value(0).toString(), query.value(2).toString());
    m_ui->labelName->setText( label );
    m_ui->labelName->setToolTip( label );

    QString publisher = query.value(3).toString();
    m_ui->labelPublisher->setText( publisher );
    m_ui->labelPublisher->setToolTip( publisher );

    QString format = query.value(1).toString();
    m_ui->labelFormat->setText( format );
    m_ui->labelFormat->setToolTip( format );
  }
}

/* end of file */