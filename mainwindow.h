#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_action_Close_triggered();
  void on_action_Open_triggered();
  void on_action_New_triggered();

private:
  bool createDatabase(const QString& filename);
  bool connectDatabase(const QString& filename);

  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
