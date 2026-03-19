#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mde.h"

QT_BEGIN_NAMESPACE
namespace Ui { class mdeClass; };
QT_END_NAMESPACE

class mde : public QMainWindow
{
    Q_OBJECT

public:
    mde(QWidget *parent = nullptr);
    ~mde();

private:
    Ui::mdeClass *ui;
};

