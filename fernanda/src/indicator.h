// indicator.h, Fernanda

#pragma once

#include "uni.h"

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QWidget>

class Indicator : public QWidget
{
    Q_OBJECT

public:
    const QString objectName = QStringLiteral("indicator");

    Indicator(QWidget* parent = nullptr)
    {
        separator->setText("/");
        positions->setGraphicsEffect(opacity(0.8));
        separator->setGraphicsEffect(opacity(0.3));
        counts->setGraphicsEffect(opacity(0.8));
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
        for (auto& widget : { positions, separator, counts })
        {
            widget->setObjectName(objectName);
            layout->addWidget(widget);
        }
        connect(this, &Indicator::toggled, this, [&]()
            {
                if (positions->isHidden() || counts->isHidden())
                    separator->hide();
                else
                    separator->show();
            });
    }

    struct Has {
        bool linePos = true;
        bool colPos = true;
        bool lineCount = true;
        bool wordCount = true;
        bool charCount = true;
    } has;

public slots:
    void updatePositions(const int cursorBlockNumber, const int cursorPosInBlock)
    {
        if (!hideOrShow(positions, has.linePos, has.colPos)) return;
        QStringList elements;
        if (has.linePos)
            elements << "ln " + QString::number(cursorBlockNumber + 1);
        if (has.colPos)
            elements << "col " + QString::number(cursorPosInBlock + 1);
        positions->setText(elements.join(", "));
    }

    void updateCounts(const QString text, const int blockCount)
    {
        if (!hideOrShow(counts, has.lineCount, has.wordCount, has.charCount)) return;
        const auto word_count = text.split(Uni::regex(Uni::Re::Split), Qt::SkipEmptyParts).count();
        const auto char_count = text.count();
        QStringList elements;
        if (has.lineCount)
            elements << QString::number(blockCount) + " lines";
        if (has.wordCount)
            elements << QString::number(word_count) + " words";
        if (has.charCount)
            elements << QString::number(char_count) + " chars";
        counts->setText(elements.join(", "));
    }

    void updateSelection(const QString selectedText, const int lineCount)
    {
        updateCounts(selectedText, lineCount);
    }

private:
    QHBoxLayout* layout = new QHBoxLayout(this);
    QLabel* positions = new QLabel(this);
    QLabel* separator = new QLabel(this);
    QLabel* counts = new QLabel(this);

    bool hideOrShow(QLabel* label, bool feature1, bool feature2, bool feature3 = false)
    {
        auto result = false;
        if (!feature1 && !feature2 && !feature3)
        {
            label->hide();
            toggled();
        }
        else
        {
            label->show();
            toggled();
            result = true;
        }
        return result;
    }

    QGraphicsOpacityEffect* opacity(double qreal)
    {
        auto result = new QGraphicsOpacityEffect(this);
        result->setOpacity(qreal);
        return result;
    }

signals:
    void toggled();
};

// indicator.h, Fernanda
