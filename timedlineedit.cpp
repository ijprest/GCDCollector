/*SDOC:**********************************************************************

	File:			timedlineedit.cpp

	Action:		Implementation of the TimedLineEdit widget.

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
#include <QTimerEvent>
#include "timedlineedit.h"

/////////////////////////////////////////////////////////////////////////////
// TimedLineEdit widget class
/////////////////////////////////////////////////////////////////////////////

/*SDOC:**********************************************************************

	Name:			TimedLineEdit::TimedLineEdit

	Action:		Constructor

**********************************************************************:EDOC*/
TimedLineEdit::TimedLineEdit(QWidget* parent)
  : QLineEdit(parent),
    timerId(-1)
{
  connect(this, SIGNAL(textEdited(QString)), this, SLOT(startEditTimer()));
  connect(this, SIGNAL(editingFinished()), this, SLOT(stopEditTimer()));
}


/*SDOC:**********************************************************************

	Name:			TimedLineEdit::startEditTimer
						TimedLineEdit::stopEditTimer
						TimedLineEdit::timerEvent

	Action:		SLOTs called to start & stop the timer

	Comments:	Called in response to text change events from the underlying 
						LineEdit control.  When the user stops typing, we wait for 
						a short amount of time, then emit a timedEdit() SIGNAL.

**********************************************************************:EDOC*/
void TimedLineEdit::startEditTimer()
{
  killTimer(timerId);
  timerId = startTimer(500);
}

void TimedLineEdit::stopEditTimer()
{
  killTimer(timerId);
  if(this->text() != oldText)
  {
    oldText = this->text();
    this->timedEdit(oldText);
  }
}

void TimedLineEdit::timerEvent(QTimerEvent *event)
{
  if(event->timerId() == timerId)
    stopEditTimer();
}

/* end of file */