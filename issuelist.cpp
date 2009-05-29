#include "issuelist.h"
#include <QSqlQueryModel>

IssueList::IssueList(QWidget* parent)
  : QTableView(parent),
    model(NULL)
{
}

IssueList::~IssueList()
{
  delete model;
}

void IssueList::setSeries(int seriesId)
{
  if(model) { delete model; model = NULL; }
  model = new QSqlQueryModel;
  model->setQuery(QString("SELECT Id, Number, key_date, Price, page_count, notes FROM core_issue WHERE series_id=%1 ORDER BY Number").arg(seriesId));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
}
