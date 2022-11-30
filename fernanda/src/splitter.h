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
        setCollapsible(0, true);
        setCollapsible(1, false);
        setStretchFactor(1, 100);
    }

    void loadConfig(QRect geo)
    {
        auto state = Ud::loadConfig("window", "splitter", QVariant()).toByteArray();
        if (state.isEmpty() || state.isNull())
            setSizes(QVector<int>{ static_cast<int>(geo.width() * 0.2), static_cast<int>(geo.width() * 0.8) });
        else
            restoreState(state);
    }
};

// splitter.h, fernanda