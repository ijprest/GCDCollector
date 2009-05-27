#include <QTimerEvent>
#include "timedlineedit.h"

TimedLineEdit::TimedLineEdit(QWidget* parent)
  : QLineEdit(parent),
    timerId(-1)
{
  connect(this, SIGNAL(textEdited(QString)), this, SLOT(startEditTimer()));
  connect(this, SIGNAL(editingFinished()), this, SLOT(stopEditTimer()));
}

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
  {
    stopEditTimer();
  }
}
