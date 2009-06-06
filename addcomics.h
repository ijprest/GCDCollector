#ifndef ADDCOMICS_H
#define ADDCOMICS_H

namespace Ui { class AddComics; }

class AddComics : public QDialog
{
  Q_OBJECT
public:
	AddComics(QWidget* parent = 0);
	~AddComics();

signals:
	void addItems(const QList<int>& items);

private slots:
	void addItemsClicked();

private:
  Ui::AddComics* ui;
};


#endif // ADDCOMICS_H
