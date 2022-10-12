// pane.cpp, fernanda

#include "pane.h"

Pane::Pane(QWidget* parent)
    : QTreeView(parent)
{
    setItemDelegate(paneDelegate);
    setModel(itemModel);
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers); // temp
    setExpandsOnDoubleClick(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    connect(this, &Pane::clicked, this, [&](const QModelIndex& index)
        {
            if (index.data(Qt::UserRole).toString() == "file")
                askSendToEditor(index.data(Qt::UserRole + 1).toString());
        });
}

void Pane::receiveItems(QVector<QStandardItem*> itemsVector)
{
    itemModel->clear();
    for (auto& item : itemsVector)
        itemModel->appendRow(item);
}

void Pane::receiveEditsList(QVector<QString> editedFiles)
{
    if (editedFiles == paneDelegate->paintAsEdited) return;
    paneDelegate->paintAsEdited = editedFiles;
    refresh();
}

void Pane::replyNavPrevious()
{
    nav(Nav::Prev);
}

void Pane::replyNavNext()
{
    nav(Nav::Next);
}

void Pane::contextMenuEvent(QContextMenuEvent* event)
{
    auto project = askHasProject();
    if (!project) return;
    auto menu = new QMenu(this);
    if (currentIndex().data(Qt::UserRole).toString() == "file")
    {
        auto* rename_item = new QAction(tr("&Rename (WIP)"), this);
        auto* delete_item = new QAction(tr("&Delete (WIP)"), this);
        connect(rename_item, &QAction::triggered, this, [&]() {});
        connect(delete_item, &QAction::triggered, this, [&]() {});
        menu->addAction(rename_item);
        menu->addAction(delete_item);
    }
    else
    {
        auto* new_folder = new QAction(tr("&New folder (WIP)"), this);
        auto* new_file = new QAction(tr("&New file (WIP)"), this);
        connect(new_folder, &QAction::triggered, this, [&]() {});
        connect(new_file, &QAction::triggered, this, [&]() {});
        menu->addAction(new_folder);
        menu->addAction(new_file);
    }
    menu->exec(event->globalPos());
}

void Pane::refresh()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void Pane::nav(Nav direction)
{
    QModelIndex next;
    if (direction == Nav::Prev)
        next = indexAbove(currentIndex());
    else
        next = indexBelow(currentIndex());
    if (next.isValid())
    {
        setCurrentIndex(next);
    }
    else
    {
        setCurrentIndex(model()->index(0, 0));
        auto valid = true;
        while (valid)
        {
            QModelIndex wrap_around;
            if (direction == Nav::Prev)
                wrap_around = indexBelow(currentIndex());
            else
                wrap_around = indexAbove(currentIndex());
            if (wrap_around.isValid())
                setCurrentIndex(wrap_around);
            else
                valid = false;
        }
    }
    auto type = currentIndex().data(Qt::UserRole).toString();
    auto expanded = isExpanded(currentIndex());
    if (type == "dir" && !expanded)
        expand(currentIndex());
    else
        clicked(currentIndex());
}

void Pane::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (currentIndex().data(Qt::UserRole).toString() == "dir")
        QTreeView::mouseDoubleClickEvent(event);
}

// pane.cpp, fernanda