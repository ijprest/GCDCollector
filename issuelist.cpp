#include "stdafx.h"
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
  model->setQuery(QString("SELECT Id, Number, publication_date, Price, page_count, notes FROM issues WHERE series_id=%1 ORDER BY Number").arg(seriesId));
	model->setHeaderData(0, Qt::Horizontal, "id");
	model->setHeaderData(1, Qt::Horizontal, tr("Number"));
	model->setHeaderData(2, Qt::Horizontal, tr("Date"));
	model->setHeaderData(3, Qt::Horizontal, tr("Price"));
	model->setHeaderData(4, Qt::Horizontal, tr("Pages"));
	model->setHeaderData(5, Qt::Horizontal, tr("Notes"));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
	resizeColumnsToContents();
}
