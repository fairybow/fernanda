// main.cpp, fernanda

#include "fernanda.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    Fernanda main_window;
    main_window.show();
    return application.exec();
}

// main.cpp, fernanda