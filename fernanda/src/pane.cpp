// pane.cpp, fernanda

#include "pane.h"

Pane::Pane(QWidget* parent)
    : QTreeView(parent)
{
    setObjectName("pane");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setItemDelegate(delegate);
    setModel(itemModel);
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setExpandsOnDoubleClick(true);
    setDragEnabled(true);
    viewport()->setAcceptDrops(false);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    connect(this, &Pane::clicked, this, [&](const QModelIndex& index)
        {
            if (Index::isFile(index))
                askSendToEditor(Index::key(index));
        });
    connect(this, &Pane::expanded, this, [&](const QModelIndex& index)
        {
            askSetExpansion(Index::key(index), true);
        });
    connect(this, &Pane::collapsed, this, [&](const QModelIndex& index)
        {
            askSetExpansion(Index::key(index), false);
        });
}

void Pane::nav(Nav direction)
{
    QModelIndex next;
    (direction == Nav::Previous)
        ? next = indexAbove(currentIndex())
        : next = indexBelow(currentIndex());
    if (next.isValid())
        setCurrentIndex(next);
    else
    {
        setCurrentIndex(model()->index(0, 0));
        auto valid = true;
        while (valid)
        {
            QModelIndex wrap_around;
            (direction == Nav::Previous)
                ? wrap_around = indexBelow(currentIndex())
                : wrap_around = indexAbove(currentIndex());
            if (wrap_around.isValid())
                setCurrentIndex(wrap_around);
            else
                valid = false;
        }
    }
    auto expanded = isExpanded(currentIndex());
    if (Index::isDir(currentIndex()) && !expanded)
        expand(currentIndex());
    else
        clicked(currentIndex());
}

void Pane::receiveItems(QVector<QStandardItem*> items)
{
    itemModel->clear();
    for (auto& item : items)
    {
        itemModel->appendRow(item);
        expandItems_recursor(item);
    }
}

void Pane::receiveEditsList(QVector<QString> editedFiles)
{
    if (editedFiles == delegate->paintEdited) return;
    delegate->paintEdited = editedFiles;
    refresh();
}

void Pane::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (currentIndex().model()->hasChildren())
        QTreeView::mouseDoubleClickEvent(event);
}

void Pane::dropEvent(QDropEvent* event)
{
    auto drop = dropIndicatorPosition();
    auto pivot = currentIndex();
    auto fulcrum = indexAt(event->pos());
    Io::Move pos{};
    switch (drop) {
    case QAbstractItemView::AboveItem:
        pos = Io::Move::Above;
        break;
    case QAbstractItemView::BelowItem:
        pos = Io::Move::Below;
        break;
    case QAbstractItemView::OnItem:
        if (Index::isDir(pivot) && Index::isFile(fulcrum)) return;
        pos = Io::Move::On;
        expand(fulcrum);
        break;
    case QAbstractItemView::OnViewport:
        pos = Io::Move::Viewport;
        break;
    }
    askDomMove(Index::key(pivot), Index::key(fulcrum), pos);
    event->ignore();
}

void Pane::contextMenuEvent(QContextMenuEvent* event)
{
    auto project = askHasProject();
    if (!project) return;
    auto& pos = event->pos();
    auto index = indexAt(pos);
    auto menu = new QMenu(this);
    auto* rename_item = new QAction(tr("&Rename"), this);
    auto* cut_item = new QAction(tr("&Cut"), this);
    auto* new_folder = new QAction(tr("&New folder"), this);
    auto* new_file = new QAction(tr("&New file"), this);
    connect(rename_item, &QAction::triggered, this, [&]()
        {
            askRenameElement(rename(), Index::key(currentIndex()));
        });
    connect(cut_item, &QAction::triggered, this, [&]()
        {
            askCutElement(Index::key(currentIndex()));
        });
    connect(new_folder, &QAction::triggered, this, [&]()
        {
            addTempItem(pos, Path::Type::Dir);
        });
    connect(new_file, &QAction::triggered, this, [&]()
        {
            addTempItem(pos, Path::Type::File);
        });
    if (Index::isFile(index))
    {
        menu->addAction(rename_item);
        menu->addSeparator();
        menu->addAction(new_file);
        menu->addSeparator();
        menu->addAction(cut_item);
    }
    else if (Index::isDir(index))
    {
        menu->addAction(rename_item);
        menu->addSeparator();
        menu->addAction(new_folder);
        menu->addAction(new_file);
        menu->addSeparator();
        menu->addAction(cut_item);
    }
    else
    {
        menu->addAction(new_folder);
        menu->addAction(new_file);
    }
    menu->exec(event->globalPos());
}

void Pane::resizeEvent(QResizeEvent* event)
{
    delegate->paneSize = event->size();
    QTreeView::resizeEvent(event);
    refresh();
}

void Pane::expandItems_recursor(QStandardItem* item)
{
    auto index = item->index();
    if (Index::isExpanded(index))
        setExpanded(index, true);
    if (item->hasChildren())
        for (auto i = 0; i < item->rowCount(); ++i)
            expandItems_recursor(item->child(i));
}

void Pane::refresh()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void Pane::addTempItem(QPoint eventPos, Path::Type type)
{
    auto temp_item = tempItem(type);
    auto parent_index = indexAt(eventPos);
    switch (type) {
    case Path::Type::Dir:
        if (parent_index.isValid() && Index::isDir(parent_index))
            itemModel->itemFromIndex(parent_index)->appendRow(temp_item);
        else
            itemModel->appendRow(temp_item);
        break;
    case Path::Type::File:
        (parent_index.isValid())
            ? itemModel->itemFromIndex(parent_index)->appendRow(temp_item)
            : itemModel->appendRow(temp_item);
        break;
    }
    if (parent_index.isValid())
        expand(parent_index);
    temp_item->setEnabled(false);
    QString parent_key;
    if (parent_index.isValid())
        parent_key = Index::key(parent_index);
    askAddElement(rename(), type, parent_key);
}

QStandardItem* Pane::tempItem(Path::Type type)
{
    QStandardItem* result = new QStandardItem();
    switch (type) {
    case Path::Type::Dir:
        result->setData("dir", Qt::UserRole);
        break;
    case Path::Type::File:
        result->setData("file", Qt::UserRole);
        break;
    }
    result->setData("Untitled", Qt::UserRole + 1);
    return result;
}

const QString Pane::rename()
{
    bool has_input = false;
    QString text = QInputDialog::getText(this, tr(nullptr), tr(nullptr), QLineEdit::Normal, nullptr, &has_input);
    if (has_input && !text.isEmpty())
        return text.replace(Uni::regex(Uni::Re::Forbidden), "_");
    return nullptr;
}

// pane.cpp, fernanda