// colorbar.cpp, Fernanda

#include "colorbar.h"

ColorBar::ColorBar(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(bar);
    bar->setAttribute(Qt::WA_TransparentForMouseEvents);
    bar->setMaximumHeight(3);
    bar->setTextVisible(false);
    bar->setRange(0, 100);
    bar->hide();
    bar->setObjectName("colorBar");
    connect(barTimer, &QTimer::timeout, this, [&]()
        {
            bar->hide();
            bar->reset();
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

void ColorBar::align(Qt::AlignmentFlag alignment)
{
    layout->setAlignment(alignment);
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
    connect(bar_fill, &QTimeLine::frameChanged, bar, &QProgressBar::setValue);
    bar_fill->setFrameRange(0, 100);
    bar->show();
    barTimer->start(1000);
    bar_fill->start();
}

void ColorBar::style(Color theme)
{
    QString style_sheet;
    switch (theme) {
    case Color::Red:
        style_sheet = Io::readFile(":/themes/bar/red.qss");
        break;
    case Color::Green:
        style_sheet = Io::readFile(":/themes/bar/green.qss");
        break;
    case Color::Pastels:
        style_sheet = Io::readFile(":/themes/bar/pastels.qss");
        break;
    }
    bar->setStyleSheet(style_sheet);
}

// colorbar.cpp, Fernanda
