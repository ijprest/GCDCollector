#ifndef ADDCOMICS_H
#define ADDCOMICS_H

namespace Ui { class AddComics; }

class AddComics : public QDialog
{
  Q_OBJECT
public:
	AddComics(QWidget* parent = 0);
	~AddComics();

private:
  Ui::AddComics* ui;
};


#endif // ADDCOMICS_H
