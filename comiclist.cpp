#include "stdafx.h"
#include "comiclist.h"
#include <QSqlQueryModel>

ComicList::ComicList(QWidget* parent)
  : QTableView(parent),
    model(NULL)
{
}

ComicList::~ComicList()
{
  delete model;
}

void ComicList::setSeries(int seriesId)
{
  if(model) { delete model; model = NULL; }
  model = new QSqlQueryModel;
  model->setQuery(QString("SELECT document.comics.id, issues.number, issues.publication_date, "
																 "document.comics.condition, document.comics.price, document.comics.store, document.comics.notes "
													"FROM document.comics "
													"INNER JOIN issues ON issues.id = document.comics.issue_id "
													"WHERE issues.series_id = %1 "
													"ORDER BY issues.sort_code;").arg(seriesId));
	model->setHeaderData(0, Qt::Horizontal, "id");
	model->setHeaderData(1, Qt::Horizontal, tr("Number"));
	model->setHeaderData(2, Qt::Horizontal, tr("Date"));
	model->setHeaderData(3, Qt::Horizontal, tr("Condition"));
	model->setHeaderData(4, Qt::Horizontal, tr("Price Paid"));
	model->setHeaderData(5, Qt::Horizontal, tr("Store"));
	model->setHeaderData(6, Qt::Horizontal, tr("Notes"));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
	resizeColumnsToContents();
}
