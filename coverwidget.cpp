/*SDOC:**********************************************************************

	File:			coverwidget.cpp

	Action:		Implementation of the CoverViewer widget.

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
#include "coverwidget.h"

/////////////////////////////////////////////////////////////////////////////
// CoverViewer widget class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			CoverViewer::CoverViewer
						CoverViewer::~CoverViewer

	Action:		Constructor / destructor

**********************************************************************:EDOC*/
CoverViewer::CoverViewer(QWidget* parent)
	: QWidget(parent),
		http(this),
		tempFile(0),
		image(0),
		scaledImage(0),
		currentId(0)
{
	setAcceptDrops(true);
	connect(&http, SIGNAL(done(bool)), this, SLOT(httpDone(bool)));
	connect(&http, SIGNAL(dataReadProgress(int,int)), this, SLOT(httpStatus(int,int)));
}

CoverViewer::~CoverViewer()
{
	http.abort();
	delete tempFile;
	delete image;
	delete scaledImage;
}


/*SDOC:**********************************************************************

	Name:			CoverViewer::dragEnterEvent
						CoverViewer::dropEvent

	Action:		Handle drag/drop operations

**********************************************************************:EDOC*/
void CoverViewer::dragEnterEvent(QDragEnterEvent* ev)
{
	if(ev->mimeData()->hasFormat("text/uri-list"))
		ev->acceptProposedAction();
}

void CoverViewer::dropEvent(QDropEvent* ev)
{
	QList<QUrl> urls = ev->mimeData()->urls();
	if(urls.isEmpty())
		return;
	getFile(urls.first());
}

bool CoverViewer::getFile(const QUrl& url)
{
	if (!url.isValid()) 
		return false;

	if(url.scheme() == "http") 
	{
		if(url.path().isEmpty())
			return false;
	
		downloadInProgress(true);

		http.abort();
		tempFile = new QTemporaryFile();
		tempFile->open();
		http.setHost(url.host(), url.port(80));
		http.get(url.path(), tempFile);
		http.close();
		return true;
	}
	else if(!url.toLocalFile().isEmpty())
	{
		// already local
		return readImage(url.toLocalFile());
	}
	return false;
}

void CoverViewer::httpDone(bool error)
{
	if(tempFile)
	{
		QString fileName = tempFile->fileName();
		tempFile->close();
		if(!error)
			readImage(fileName);
		delete tempFile;
		tempFile = NULL;
	}
	downloadInProgress(false);
}

void CoverViewer::setImageId(int id)
{
	currentId = id;

	// Free old image
	delete image;
	image = NULL;
	delete scaledImage;
	scaledImage = NULL;

	// Try to load the image from the database
	QSqlQuery query;
	query.prepare("SELECT data FROM document.images WHERE id=?;");
	query.addBindValue(currentId);
	query.exec();
	if(query.next()) // only expecting one row
	{
		QByteArray bytes = query.value(0).toByteArray();
		if(bytes.length() > 0)
		{
			image = new QImage();
			image->loadFromData(bytes);
		}
	}
	update();
}

bool CoverViewer::readImage(const QString& fileName)
{
	// Free old image
	delete image; 
	image = NULL;
	delete scaledImage;
	scaledImage = NULL;

	// Load new image
	image = new QImage(fileName);
	if(!image->isNull())
	{
		// Was able to load the image; now read it again and store 
		// it in the database.
		QFile file(fileName);
		if(file.open(QIODevice::ReadOnly))
		{
			// Delete any old image in the database
			QSqlQuery deleteQuery;
			deleteQuery.prepare("DELETE FROM document.images WHERE id=?;");
			deleteQuery.addBindValue(currentId);
			deleteQuery.exec();

			// Insert the new image
			QSqlQuery query;
			query.prepare("INSERT INTO document.images VALUES (?,?);");
			query.addBindValue(currentId);
			query.addBindValue(file.readAll());
			if(!query.exec())
				QMessageBox::critical(this, tr("Database Error"), query.lastError().text());
			file.close();
		}
	}
	update();
	return image->isNull();
}

void CoverViewer::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	if(image) 
	{
		QRect windowRect = painter.window();

		// Make sure we have a nicely scaled image to the correct size;
		// we cache the image to avoid performance problems.
		if(!scaledImage || 
			 (scaledImage->width() != windowRect.width() && scaledImage->height() != windowRect.height()) ||
			 scaledImage->width() > windowRect.width() ||
			 scaledImage->height() > windowRect.height())
		{
			delete scaledImage;
			scaledImage = new QImage(image->scaled(windowRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}

		// Compute the draw rect
		QRect drawRect = scaledImage->rect();
		drawRect.moveCenter(windowRect.center());

		// Paint the image
		painter.drawImage(drawRect, *scaledImage);
	}
}