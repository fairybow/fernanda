#pragma once

#include "pane_delegate.h"

#include <tuple>

#include <QFileInfo>
#include <QShortcut>
#include <QString>
#include <QTreeView>
//
//#include <QAbstractItemView>

class Pane : public QTreeView
{
    Q_OBJECT

public:
    Pane(QWidget* parent = nullptr);
    ~Pane() = default;

    PaneDelegate* paneDelegate = new PaneDelegate(this);

    std::tuple<QModelIndex, QString> selectedIndex;
    std::tuple<QString, QModelIndex> currentFile;

    void setup(QString path);
    void refresh();
    void clearTuples();

public slots:
    void navPrevious();
    void navNext();

private:
    QFileSystemModel* dirModel = new QFileSystemModel(this);

    void hotKeys();
    void collapseOrExpand(QModelIndex index);
    const QModelIndex navPreviousWrapAround();
    const QModelIndex navNextWrapAround();
    void navExpand();

private slots:
    void onClick(QModelIndex index);

signals:
    void previousFileIsFile(QString path);
    void pathDoesNotEqualPrevPath(QString path);
    void changePathDisplay();

};

// pane.h, fernanda