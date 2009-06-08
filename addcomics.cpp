/*SDOC:**********************************************************************

	File:			addcomics.cpp

	Action:		Implementation of the AddComics dialog.

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
#include "addcomics.h"
#include "ui_addcomics.h"

/////////////////////////////////////////////////////////////////////////////
// AddComics dialog class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			AddComics::AddComics
						AddComics::~AddComics

	Action:		Constructor / Destructor

**********************************************************************:EDOC*/
AddComics::AddComics(QWidget* parent)
	: QDialog(parent),
		ui(new Ui::AddComics)
{
	ui->setupUi(this);
	connect(ui->addSelectedItemsButton, SIGNAL(clicked()), this, SLOT(addItemsClicked()));
}

AddComics::~AddComics()
{
	delete ui;
}


/*SDOC:**********************************************************************

	Name:			AddComics::addItemsClicked

	Action:		Builds a list of selected items, and emits the "addItems" 
						signal.

**********************************************************************:EDOC*/
void AddComics::addItemsClicked()
{
	if( ui->tableView->selectionModel() && ui->tableView->selectionModel()->hasSelection() )
	{
		QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();
		QList<int> items;
		for(QModelIndexList::iterator i = rows.begin(); i != rows.end(); ++i)
			items << (*i).sibling((*i).row(),0).data().toInt();
		addItems(items);
	}
}

/* end of file */