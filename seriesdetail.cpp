#include <QSqlRecord>
#include <QSqlQuery>
#include "seriesdetail.h"
#include "ui_seriesdetail.h"

SeriesDetail::SeriesDetail(QWidget *parent) :
  QGroupBox(parent),
  m_ui(new Ui::SeriesDetail)
{
  m_ui->setupUi(this);
}

SeriesDetail::~SeriesDetail()
{
  delete m_ui;
}

void SeriesDetail::setSeries(int seriesId)
{
  QSqlQuery query;
  query.exec(QString("SELECT name, format, year_began, publisher_name FROM core_series WHERE core_series.id = %1;").arg(seriesId));
  if(query.next()) // only expecting one row
  {
    m_ui->labelName->setText( QString("%1 (%2)").arg(query.value(0).toString(), query.value(2).toString()) );
    m_ui->labelPublisher->setText( query.value(3).toString() );
    m_ui->labelFormat->setText( query.value(1).toString() );
  }
}
