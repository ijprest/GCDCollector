#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

namespace Ui { class MainWindow; }
class AddComics;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget* parent = 0);
  ~MainWindow();

private slots:;
  void closeDatabase();
  void openDatabase();
  void newDatabase();
	void addComics();
	void about();
	void addItems(const QList<int>& items);

private:
  bool createDatabase(const QString& filename);
  bool connectDatabase(const QString& filename);

	AddComics* addComicsDialog;
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
