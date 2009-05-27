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
  model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM core_series WHERE id=%1").arg(seriesId));
  model->setHeaderData(0,Qt::Horizontal,"Id");
  model->setHeaderData(1,Qt::Horizontal,tr("Name"));

  setModel(model);
 // setColumnHidden(0, true);
 // resizeRowsToContents();
 // connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChange(QModelIndex)));
}
