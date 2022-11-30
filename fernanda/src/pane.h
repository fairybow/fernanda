// pane.h, fernanda

#pragma once

#include "delegate.h"
#include "io.h"

#include <QAbstractItemView>
#include <QAction>
#include <QContextMenuEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>

class Pane : public QTreeView
{
    Q_OBJECT

public:
    Pane(QWidget* parent = nullptr);

    enum class Nav {
        Next,
        Previous
    };

    void nav(Nav direction);

public slots:
    void receiveItems(QVector<QStandardItem*> items);
    void receiveEditsList(QVector<QString> editedFiles);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QStandardItemModel* itemModel = new QStandardItemModel(this);
    PaneDelegate* paneDelegate = new PaneDelegate(this);

    void expandItems_recursor(QStandardItem* item);
    void refresh();
    void addTempItem(QPoint eventPos, Path::Type type);
    QStandardItem* makeTempItem(Path::Type type);
    const QString renameItem();

signals:
    bool askHasProject();
    void askSendToEditor(QString key);
    void askDomMove(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void askAddElement(QString newName, Path::Type type, QString parentKey);
    void askRenameElement(QString newName, QString key);
    void askSetExpansion(QString key, bool isExpanded);
};

// pane.h, fernanda