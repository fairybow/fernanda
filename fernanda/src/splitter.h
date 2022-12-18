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
    const QString objectName = QStringLiteral("splitter");

    Splitter(QWidget* parent = nullptr)
    {
        setObjectName(objectName);
        connect(this, &QSplitter::splitterMoved, this, [&]()
            {
                Ud::saveConfig(Ud::window, objectName, saveState());
            });
    }

    void addWidgets(QVector<QWidget*> widgets)
    {
        for (auto& widget : widgets)
            addWidget(widget);
        setCollapsible(0, true);
        auto editor = 1;
        setCollapsible(editor, false);
        setStretchFactor(editor, 100);
    }

    void loadConfig(QRect geo)
    {
        auto state = Ud::loadConfig(Ud::window, objectName, QVariant()).toByteArray();
        if (state.isEmpty() || state.isNull())
            setSizes(QVector<int>{ static_cast<int>(geo.width() * 0.2), static_cast<int>(geo.width() * 0.8) });
        else
            restoreState(state);
    }
};

// splitter.h, fernanda