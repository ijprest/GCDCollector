#ifndef SERIESLIST_H
#define SERIESLIST_H
#include <QTableView>

class QSqlQueryModel;

class SeriesList : public QTableView
{
  Q_OBJECT
public:
  SeriesList(QWidget* parent);
  ~SeriesList();

// interface
signals:
  void seriesSelected(int id);
public slots:
  void filterList(const QString& filter);

// implementation
private slots:
  void selectionChange(const QModelIndex& index);
private:
  QSqlQueryModel* model;
};

#endif // SERIESLIST_H
