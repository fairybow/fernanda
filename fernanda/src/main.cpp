// main.cpp, fernanda

#include "fernanda.h"

int main(int argc, char *argv[])
{
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setDesktopSettingsAware(true);
    QApplication app(argc, argv);
    Fernanda window;
    {
        QFont font;
        font.setPointSizeF(9);
        app.setFont(font);
    }
    window.show();
    return app.exec();
}

// main.cpp, fernanda