#include "mde.h"

mde::mde(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mdeClass())
{
    ui->setupUi(this);
}

mde::~mde()
{
    delete ui;
}

