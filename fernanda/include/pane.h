#pragma once

#include "pane_delegate.h"

#include "tuple"

#include <QFileInfo>
#include <QShortcut>
#include <QString>
#include <QTreeView>

using namespace std;

class Pane : public QTreeView
{
    Q_OBJECT

public:
    Pane(QWidget* parent = nullptr);
    ~Pane() = default;

    PaneDelegate* paneDelegate = new PaneDelegate(this);

    tuple<QModelIndex, QString> selectedIndex;
    tuple<QString, QModelIndex> currentFile;

    void setup(QString path);
    void refresh();

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