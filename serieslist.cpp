#include "stdafx.h"
#include "serieslist.h"

SeriesList::SeriesList(QWidget* parent)
  : QTableView(parent),
		onlyShowOwned(false),
    model(NULL)
{
}

SeriesList::~SeriesList()
{
  delete model;
}

void SeriesList::filterList(const QString& filter)
{
  if(model) { delete model; model = NULL; }
  model = new QSqlQueryModel;

	if(onlyShowOwned)
	{
		model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM series "
														"WHERE name LIKE '%%1%' AND id IN ("
															"SELECT DISTINCT series_id FROM issues WHERE issues.id IN (SELECT issue_id FROM document.comics)"
														")"
														"ORDER BY Name").arg(filter));
	}
	else
	{
		model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM series "
														"WHERE name LIKE '%%1%' "
														"ORDER BY Name").arg(filter));
	}
  model->setHeaderData(0,Qt::Horizontal,"Id");
  model->setHeaderData(1,Qt::Horizontal,tr("Name"));

  setModel(model);
  setColumnHidden(0, true); // Hide the "id" column
  connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChange(QModelIndex)));
}

void SeriesList::selectionChange(const QModelIndex& index)
{
  // Retrieve the series ID (column 0 in the selected row)
  int seriesId = index.sibling(index.row(),0).data().toInt();
  seriesSelected(seriesId);
}
