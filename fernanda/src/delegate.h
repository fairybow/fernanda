// delegate.h, fernanda

#pragma once

#include <QColor>
#include <QFont>
#include <QModelIndex>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QVector>
#include <QWidget>

class PaneDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QVector<QString> paintAsEdited;

private:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        auto type = index.data(Qt::UserRole).toString();
        auto path = index.data(Qt::UserRole + 1).toString();
        auto name = index.data(Qt::UserRole + 2).toString();
        QFont font;
        QFont italics;
        italics.setItalic(true);
        painter->setFont(font);
        auto opt_r = option.rect;
        auto r_h = opt_r.height();
        auto r_w = opt_r.width();
        auto r_t = opt_r.top();
        auto r_l = opt_r.left();
        auto opt_s = QSize(r_h, r_h);
        auto s_h = opt_s.height();
        auto s_w = opt_s.width();
        auto icon_r = QRect((r_l - s_w + 15), r_t, s_w, s_h);
        auto text_r = QRect((r_l + 16), (r_t + 2), r_w, r_h);
        auto highlight_r = QRect(5, r_t, (r_w + 50), r_h);
        if (type == "dir")
        {
            if (option.state & QStyle::State_Open)
                painter->drawText(icon_r, "\U0001F4C2");
            else
                painter->drawText(icon_r, "\U0001F4C1");
        }
        else if (type == "file")
            painter->drawText(icon_r, "\U0001F4C4");
        else
            painter->drawText(icon_r, "\U00002754");
        painter->drawText(text_r, name);
        for (auto& entry : paintAsEdited)
            if (path == entry)
            {
                painter->eraseRect(text_r);
                painter->setFont(italics);
                painter->drawText(text_r, "*" + name);
            }
        if (option.state & QStyle::State_MouseOver || option.state & QStyle::State_Selected)
            painter->fillRect(highlight_r, QColor(0, 0, 0, 33));
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
};

// delegate.h, fernanda