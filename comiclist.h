#ifndef COMICLIST_H
#define COMICLIST_H
#include <QTableView>

class QSqlQueryModel;

class ComicList : public QTableView
{
  Q_OBJECT
public:
  ComicList(QWidget* parent);
  ~ComicList();

// interface
public slots:
  void setSeries(int seriesId);

// implementation
private:
  QSqlQueryModel* model;
};

#endif // COMICLIST_H
