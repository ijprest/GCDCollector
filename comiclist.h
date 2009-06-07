#ifndef COMICLIST_H
#define COMICLIST_H
#include <QTableView>
#include <QSqlQueryModel>

class ComicDataModel : public QSqlQueryModel
{
public:
	ComicDataModel(int seriesId, QObject *parent = 0);
	Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;

private:
	void refresh();
	int seriesId;

	struct ColumnDef
	{
		QString dbName;
		QString uiName;
		bool isEditable;
		QVariant::Type type;
	};
	static ColumnDef columns[];
};


class ComicList : public QTableView
{
  Q_OBJECT
public:
  ComicList(QWidget* parent);
  ~ComicList();

// interface
public slots:
  void setSeries(int seriesId);

// implementation
private:
  QSqlQueryModel* model;
};

#endif // COMICLIST_H
