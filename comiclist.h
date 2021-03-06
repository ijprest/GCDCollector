/*SDOC:**********************************************************************

	File:			comiclist.h

	Action:		Declaration of the ComicList TableView.

	Copyright � 2009, Ian Prest
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
#ifndef COMICLIST_H
#define COMICLIST_H
#include <QTableView>
#include <QSqlQueryModel>

/////////////////////////////////////////////////////////////////////////////
// ComicList class
/////////////////////////////////////////////////////////////////////////////
class ComicList : public QTableView
{
  Q_OBJECT
public:
  ComicList(QWidget* parent);
  ~ComicList();

// interface
signals:
	void selectionChanged(int issueId);

public slots:
  void setSeries(int seriesId);
	void setShowOwned(bool show);
	void setShowWanted(bool show);
	void setShowSold(bool show);
	void setShowUntracked(bool show);

	void cut();
	void copy();
	void paste();
	void del();
	void duplicate();

	void refresh();

// implementation
private slots:
  void selectionChange(const QModelIndex& index);
private:
	void setModel(QAbstractItemModel* model);
  QAbstractItemModel* model_;
	int seriesId;
	bool showOwned, showWanted, showSold, showUntracked;
};

#endif // COMICLIST_H
