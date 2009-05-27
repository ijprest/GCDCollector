#ifndef TIMEDLINEEDIT_H
#define TIMEDLINEEDIT_H

#include <QtGui/QLineEdit>

class TimedLineEdit : public QLineEdit
{
  Q_OBJECT
public:
  TimedLineEdit(QWidget* parent);
signals:
  void timedEdit(const QString& text);
private slots:
  void startEditTimer();
  void stopEditTimer();
protected:
  void timerEvent(QTimerEvent *event);
private:
  int timerId;
  QString oldText;
};

#endif // TIMEDLINEEDIT_H
