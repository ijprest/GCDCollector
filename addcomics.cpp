#include "stdafx.h"
#include "addcomics.h"
#include "ui_addcomics.h"

AddComics::AddComics(QWidget* parent)
	: QDialog(parent),
		ui(new Ui::AddComics)
{
	ui->setupUi(this);
}

AddComics::~AddComics()
{
	delete ui;
}

