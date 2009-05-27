#include "serieslist.h"
#include <QSqlQueryModel>

SeriesList::SeriesList(QWidget* parent)
  : QTableView(parent),
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
  model->setQuery(QString("SELECT Id, (Name||' ('||year_began||')') AS Name FROM core_series WHERE name LIKE '%%1%' ORDER BY Name").arg(filter));
  model->setHeaderData(0,Qt::Horizontal,"Id");
  model->setHeaderData(1,Qt::Horizontal,tr("Name"));

  setModel(model);
  setColumnHidden(0, true);
  resizeRowsToContents();
  connect(selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(selectionChange(QModelIndex)));
}

void SeriesList::selectionChange(const QModelIndex& index)
{
  // Retrieve the series ID (column 0 in the selected row)
  int seriesId = index.sibling(index.row(),0).data().toInt();
  seriesSelected(seriesId);
}
