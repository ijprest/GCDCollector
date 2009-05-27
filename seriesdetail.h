#ifndef SERIESDETAIL_H
#define SERIESDETAIL_H

#include <QtGui/QGroupBox>

namespace Ui {
    class SeriesDetail;
}

class SeriesDetail : public QGroupBox {
    Q_OBJECT
    Q_DISABLE_COPY(SeriesDetail)
public:
    explicit SeriesDetail(QWidget *parent = 0);
    virtual ~SeriesDetail();

public slots:
  void setSeries(int seriesId);

private:
    Ui::SeriesDetail *m_ui;
};

#endif // SERIESDETAIL_H
