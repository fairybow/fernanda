// bar.h, fernanda

#pragma once

#include "io.h"

#include <QProgressBar>
#include <QString>
#include <QTimeLine>
#include <QTimer>
#include <QWidget>

class ColorBar : public QProgressBar
{
    Q_OBJECT

public:
    ColorBar(QWidget* parent = nullptr);
    ~ColorBar() = default;

    void delayedStartUp();
    void pastels();
    void green();
    void red();

public slots:
    void toggleSelf(bool checked);

private:
    QTimer* barTimer = new QTimer(this);

    enum class Color {
        None = 0,
        Green,
        Red,
        Pastels
    };

    bool hasColorBar = true;

    void run(Color theme = Color::None);
    void style(Color theme);
};

// bar.h, fernanda