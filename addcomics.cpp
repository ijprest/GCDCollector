#include "stdafx.h"
#include "addcomics.h"
#include "ui_addcomics.h"

AddComics::AddComics(QWidget* parent)
	: QDialog(parent),
		ui(new Ui::AddComics)
{
	ui->setupUi(this);
	connect(ui->addSelectedItemsButton, SIGNAL(clicked()), this, SLOT(addItemsClicked()));
}

AddComics::~AddComics()
{
	delete ui;
}

void AddComics::addItemsClicked()
{
	if( ui->tableView->selectionModel() && ui->tableView->selectionModel()->hasSelection() )
	{
		QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();
		QList<int> items;
		for(QModelIndexList::iterator i = rows.begin(); i != rows.end(); ++i)
			items << (*i).sibling((*i).row(),0).data().toInt();
		addItems(items);
	}
}