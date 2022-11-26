// colorbar.h, fernanda

#pragma once

#include "io.h"

#include <QProgressBar>
#include <QString>
#include <QTimeLine>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class ColorBar : public QWidget
{
    Q_OBJECT

public:
    ColorBar(QWidget* parent = nullptr);

    void delayedStartUp();
    void pastels();
    void green();
    void red();
    void align(Qt::AlignmentFlag alignment);

public slots:
    void toggleSelf(bool checked);

private:
    QProgressBar* bar = new QProgressBar(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
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

// colorbar.h, fernanda