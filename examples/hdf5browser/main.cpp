#include "h5browserwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    H5BrowserWidget w;
    w.show();
    return a.exec();
}
