#pragma once

#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFont>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

class PaneDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QModelIndexList dirtyIndexes;

private:
    QFileSystemModel* dirModel = new QFileSystemModel(this);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// pane_delegate.h, fernanda