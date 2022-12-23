// delegate.h, fernanda

#pragma once

#include "index.h"
#include "uni.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <QSize>
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

    QVector<QString> paintEdited;
    QSize paneSize;

private:
    struct Geometry {
        QRect icon;
        QRect text;
        QRect highlight;
    };

    const QColor highlight() const
    {
        return QColor(0, 0, 0, 33);
    }

    const Geometry getRectSizes(const QStyleOptionViewItem& option) const
    {
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
        auto highlight_r = QRect(0, r_t, paneSize.width(), r_h);
        return Geometry{ icon_r, text_r, highlight_r };
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        auto geo = getRectSizes(option);
        editor->setGeometry(geo.text);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        auto item_key = Index::key(index);
        auto name = Index::name(index);
        auto geo = getRectSizes(option);
        auto dirty = false;
        QFont font;
        QFont italics;
        italics.setItalic(true);
        painter->setFont(font);
        if (option.state & QStyle::State_MouseOver || option.state & QStyle::State_Selected)
            painter->fillRect(geo.highlight, highlight());
        if (Index::isDir(index))
        {
            if (option.state & QStyle::State_Open)
                painter->drawText(geo.icon, Uni::ico(Uni::Ico::FolderOpen));
            else
                painter->drawText(geo.icon, Uni::ico(Uni::Ico::FolderClosed));
        }
        else if (Index::isFile(index))
            painter->drawText(geo.icon, Uni::ico(Uni::Ico::File));
        else
            painter->drawText(geo.icon, Uni::ico(Uni::Ico::QuestionMark));
        for (auto& entry : paintEdited)
            if (item_key == entry)
                dirty = true;
        if (dirty)
        {
            painter->setFont(italics);
            //name = name + "*";
        }
        painter->drawText(geo.text, name);
    }
};

// delegate.h, fernanda