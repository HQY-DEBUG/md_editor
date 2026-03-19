#include "mde.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    mde window;
    window.show();
    return app.exec();
}
