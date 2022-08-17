#include "fernanda.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Fernanda w;
    w.show();
    return a.exec();
}

// main.cpp, fernanda