// splitter.h, fernanda

#pragma once

#include "userdata.h"

#include <QByteArray>
#include <QSplitter>
#include <QVector>
#include <QWidget>

class Splitter : public QSplitter
{
    Q_OBJECT

public:
    Splitter(QWidget* parent = nullptr)
    {
        setCollapsible(0, true);
        setCollapsible(1, false);
        setStretchFactor(1, 100);
        setObjectName("splitter");
        connect(this, &QSplitter::splitterMoved, this, [&]()
            {
                Ud::saveConfig("window", "splitter", saveState());
            });
    }

    void addWidgets(QVector<QWidget*> widgets)
    {
        for (auto& widget : widgets)
            addWidget(widget);
    }

    void loadConfig()
    {
        auto state = Ud::loadConfig("window", "splitter").toByteArray();
        restoreState(state);
    }
};

// splitter.h, fernanda