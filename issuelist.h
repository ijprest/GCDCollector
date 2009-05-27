#ifndef ISSUELIST_H
#define ISSUELIST_H
#include <QTableView>

class QSqlQueryModel;

class IssueList : public QTableView
{
  Q_OBJECT
public:
  IssueList(QWidget* parent);
  ~IssueList();

// interface
public slots:
  void setSeries(int seriesId);

// implementation
private:
  QSqlQueryModel* model;
};

#endif // ISSUELIST_H
