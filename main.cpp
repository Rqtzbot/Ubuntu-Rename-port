#include "port.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    port w;
    w.show();

    return a.exec();
}
