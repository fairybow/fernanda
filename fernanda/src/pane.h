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
        Prev,
        Next
    };

    void nav(Nav direction);

public slots:
    void receiveInitExpansions(QVector<QString> initExpansions);
    void receiveItems(QVector<QStandardItem*> items);
    void receiveEditsList(QVector<QString> editedFiles);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    QStandardItemModel* itemModel = new QStandardItemModel(this);
    PaneDelegate* paneDelegate = new PaneDelegate(this);

    QVector<QString> expanded_metaDoc;

    void expandItem_recursor(QStandardItem* item);
    void refresh();
    void addTempItem(QPoint eventPos, Path::Type type);
    QStandardItem* makeTempItem(Path::Type type);
    const QString renameItem(); // persistentEditor or input dialog?

signals:
    bool askHasProject();
    void askSendToEditor(QString key);
    void askDomMove(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void askAddElement(QString newName, Path::Type type, QString parentKey);
    void askRenameElement(QString newName, QString key);
};

// pane.h, fernanda