#include "pane.h"

Pane::Pane(QWidget* parent)
    : QTreeView(parent)
{
    setItemDelegate(paneDelegate);
    paneDelegate->setObjectName("paneDelegate");
    dirModel->setReadOnly(true);
    hotKeys();
    connect(this, &Pane::clicked, this, &Pane::onClick);
}

void Pane::setup(QString path)
{
    dirModel->setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::AllDirs);
    dirModel->setRootPath(path);
    setModel(dirModel);
    setRootIndex(dirModel->setRootPath(path));
    for (auto int_column = 1; int_column < dirModel->columnCount(); ++int_column)
        hideColumn(int_column);
    setHeaderHidden(true);
}

void Pane::hotKeys()
{
    auto* ctrlPageUp = new QShortcut(this);
    auto* ctrlPageDown = new QShortcut(this);
    ctrlPageUp->setKey(Qt::CTRL | Qt::Key_PageUp);
    ctrlPageDown->setKey(Qt::CTRL | Qt::Key_PageDown);
    connect(ctrlPageUp, &QShortcut::activated, this, &Pane::navPrevious);
    connect(ctrlPageDown, &QShortcut::activated, this, &Pane::navNext);
}

void Pane::refresh()
{
    dataChanged(QModelIndex(), QModelIndex());
}

void Pane::clearTuples()
{
    selectedIndex = tuple<QModelIndex, QString>(QModelIndex(), ""); // idk about this one boys
    currentFile = tuple<QString, QModelIndex>("", QModelIndex());
}

void Pane::onClick(QModelIndex index)
{
    const auto& prev_selection = get<0>(selectedIndex);
    const auto& prev_selection_path = get<1>(selectedIndex);
    const auto& prev_file_path = get<0>(currentFile);
    const auto& prev_file_index = get<1>(currentFile);
    const QFileInfo check_prev_file_path(prev_file_path);

    const auto next_selection = index;
    const auto next_selection_path = dirModel->fileInfo(index).absoluteFilePath();
    const QFileInfo check_next_sel_path(next_selection_path);

    if (prev_selection != next_selection)
    {
        selectedIndex = tuple<QModelIndex, QString>(next_selection, next_selection_path);

        if (check_next_sel_path.isFile())
        {
            if (check_prev_file_path.isFile())
                previousFileIsFile(prev_file_path);

            const auto file_path = dirModel->fileInfo(index).absoluteFilePath();
            const auto file_index = index;
            const QFileInfo check_file_path(file_path);

            if (file_path != prev_file_path)
            {
                currentFile = tuple<QString, QModelIndex>(file_path, file_index);
                pathDoesNotEqualPrevPath(file_path);
            }
            else if (file_path == prev_file_path)
            {
                //
            }
        }
        else if (check_next_sel_path.isDir())
        {
            collapseOrExpand(next_selection);
        }
        changePathDisplay();
    }
    else if (prev_selection == next_selection && check_next_sel_path.isDir())
    {
        collapseOrExpand(next_selection);
    }
    else if (prev_selection == next_selection && check_next_sel_path.isFile())
    {
        //
    }
}

void Pane::collapseOrExpand(QModelIndex index)
{
    if (isExpanded(index))
        collapse(index);
    else
        expand(index);
}

void Pane::navPrevious()
{
    if (currentIndex().isValid())
    {
        if (indexAbove(currentIndex()).isValid())
        {
            auto prev_index = indexAbove(currentIndex());
            setCurrentIndex(prev_index);
        }
        else
        {
            setCurrentIndex(navPreviousWrapAround());
        }
    }
    else
    {
        setCurrentIndex(navPreviousWrapAround());
    }
    onClick(currentIndex());
    navExpand();
}

void Pane::navNext()
{
    if (currentIndex().isValid())
    {
        if (indexBelow(currentIndex()).isValid())
        {
            auto next_index = indexBelow(currentIndex());
            setCurrentIndex(next_index);
        }
        else
        {
            setCurrentIndex(navNextWrapAround());
        }
    }
    else
    {
        setCurrentIndex(navNextWrapAround());
    }
    onClick(currentIndex());
    navExpand();
}

const QModelIndex Pane::navPreviousWrapAround()
{
    setCurrentIndex(model()->index(0, 0));
    auto valid = true;
    while (valid == true)
    {
        auto index_below = indexBelow(currentIndex());
        if (index_below.isValid())
            setCurrentIndex(index_below);
        else
            valid = false;
    }
    return currentIndex();
}

const QModelIndex Pane::navNextWrapAround()
{
    setCurrentIndex(navPreviousWrapAround());
    auto valid = true;
    while (valid == true)
    {
        auto index_above = indexAbove(currentIndex());
        if (index_above.isValid())
            setCurrentIndex(index_above);
        else
            valid = false;
    }
    return currentIndex();
}

void Pane::navExpand()
{
    if (!isExpanded(currentIndex()))
        expand(currentIndex());
}

// pane.cpp, fernanda