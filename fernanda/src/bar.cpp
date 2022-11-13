// bar.cpp, fernanda

#include "bar.h"

ColorBar::ColorBar(QWidget* parent)
    : QProgressBar(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setMaximumHeight(3);
    setTextVisible(false);
    setRange(0, 100);
    hide();
    setObjectName("colorBar");
    connect(barTimer, &QTimer::timeout, this, [&]()
        {
            hide();
            reset();
        });
}

void ColorBar::delayedStartUp()
{
    QTimer::singleShot(1500, this, [&]() { pastels(); });
}

void ColorBar::pastels()
{
    run(Color::Pastels);
}

void ColorBar::green()
{
    run(Color::Green);
}

void ColorBar::red()
{
    run(Color::Red);
}

void ColorBar::toggleSelf(bool checked)
{
    hasColorBar = checked;
}

void ColorBar::run(Color theme)
{
    if (theme == Color::None) return;
    if (!hasColorBar) return;
    style(theme);
    auto* bar_fill = new QTimeLine(125, this);
    connect(bar_fill, &QTimeLine::frameChanged, this, &QProgressBar::setValue);
    bar_fill->setFrameRange(0, 100);
    show();
    barTimer->start(1000);
    bar_fill->start();
}

void ColorBar::style(Color theme)
{
    QString style_sheet;
    switch (theme) {
    case Color::Red:
        style_sheet = Io::readFile(":\\themes\\bar\\red.qss");
        break;
    case Color::Green:
        style_sheet = Io::readFile(":\\themes\\bar\\green.qss");
        break;
    case Color::Pastels:
        style_sheet = Io::readFile(":\\themes\\bar\\pastels.qss");
        break;
    }
    setStyleSheet(style_sheet);
}

// bar.cpp, fernanda