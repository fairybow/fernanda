// pane.cpp, fernanda

#include "pane.h"

Pane::Pane(QWidget* parent)
    : QTreeView(parent)
{
    setObjectName("pane");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setItemDelegate(paneDelegate);
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
            auto key = Index::key(index);
            Io::amendVector(expanded_metaDoc, key, Io::AmendVector::Add);
        });
    connect(this, &Pane::collapsed, this, [&](const QModelIndex& index)
        {
            auto key = Index::key(index);
            Io::amendVector(expanded_metaDoc, key, Io::AmendVector::Remove);
        });
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
    auto expanded = isExpanded(currentIndex());
    if (Index::isDir(currentIndex()) && !expanded)
        expand(currentIndex());
    else
        clicked(currentIndex());
}

void Pane::receiveInitExpansions(QVector<QString> initExpansions)
{
    expanded_metaDoc << initExpansions;
}

void Pane::receiveItems(QVector<QStandardItem*> items)
{
    itemModel->clear();
    for (auto& item : items)
    {
        itemModel->appendRow(item);
        expandItem_recursor(item);
    }
}

void Pane::receiveEditsList(QVector<QString> editedFiles)
{
    if (editedFiles == paneDelegate->paintAsEdited) return;
    paneDelegate->paintAsEdited = editedFiles;
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
    // event accept/ignore? deal with early return?
    askDomMove(Index::key(pivot), Index::key(fulcrum), pos);
}

void Pane::contextMenuEvent(QContextMenuEvent* event)
{
    auto project = askHasProject();
    if (!project) return;
    auto& pos = event->pos();
    auto menu = new QMenu(this);
    auto* rename_item = new QAction(tr("&Rename"), this);
    auto* delete_item = new QAction(tr("&Delete (WIP)"), this);
    auto* new_folder = new QAction(tr("&New folder"), this);
    auto* new_file = new QAction(tr("&New file"), this);
    connect(rename_item, &QAction::triggered, this, [&]()
        {
            auto name = renameItem();
            auto key = Index::key(currentIndex());
            askRenameElement(name, key);
        });
    connect(delete_item, &QAction::triggered, this, [&]() {});
    connect(new_folder, &QAction::triggered, this, [&]()
        {
            addTempItem(pos, Path::Type::Dir);
        });
    connect(new_file, &QAction::triggered, this, [&]()
        {
            addTempItem(pos, Path::Type::File);
        });
    if (Index::isFile(indexAt(pos)))
    {
        menu->addAction(rename_item);
        menu->addSeparator();
        menu->addAction(new_file);
        menu->addSeparator();
        menu->addAction(delete_item);
    }
    else if (Index::isDir(indexAt(pos)))
    {
        menu->addAction(rename_item);
        menu->addSeparator();
        menu->addAction(new_folder);
        menu->addAction(new_file);
        menu->addSeparator();
        menu->addAction(delete_item);
    }
    else
    {
        menu->addAction(new_folder);
        menu->addAction(new_file);
    }
    menu->exec(event->globalPos());
}

void Pane::expandItem_recursor(QStandardItem* item)
{
    if (expanded_metaDoc.contains(Index::key(item->index())))
        setExpanded(item->index(), true);
    if (item->hasChildren())
        for (auto i = 0; i < item->rowCount(); ++i)
            expandItem_recursor(item->child(i));
}

void Pane::refresh()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void Pane::addTempItem(QPoint eventPos, Path::Type type)
{
    auto temp_item = makeTempItem(type);
    auto parent_index = indexAt(eventPos);
    switch (type) {
    case Path::Type::Dir:
        if (parent_index.isValid() && Index::isDir(parent_index))
            itemModel->itemFromIndex(parent_index)->appendRow(temp_item);
        else
            itemModel->appendRow(temp_item);
        break;
    case Path::Type::File:
        if (parent_index.isValid())
            itemModel->itemFromIndex(parent_index)->appendRow(temp_item);
        else
            itemModel->appendRow(temp_item);
        break;
    }
    if (parent_index.isValid())
        expand(parent_index);
    temp_item->setEnabled(false);
    //openPersistentEditor(itemModel->indexFromItem(temp_item));
    auto name = renameItem();
    QString parent_key;
    if (parent_index.isValid())
        parent_key = Index::key(parent_index);
    askAddElement(name, type, parent_key);
}

QStandardItem* Pane::makeTempItem(Path::Type type)
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

const QString Pane::renameItem()
{
    bool has_input = false;
    QRegularExpression regex(Uni::regex.forbidden);
    QString text = QInputDialog::getText(this, tr(""), tr(""), QLineEdit::Normal, nullptr, &has_input);
    if (has_input && !text.isEmpty())
        return text.replace(regex, "_");
    return nullptr;
}

// pane.cpp, fernanda