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
#include <QResizeEvent>
#include <QScrollBar>
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

    const QVector<QString> devGetEditedKeys();
    void nav(Nav direction);

public slots:
    void receiveItems(QVector<QStandardItem*> items);
    void receiveEditsList(QVector<QString> editedFiles);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QStandardItemModel* itemModel = new QStandardItemModel(this);
    PaneDelegate* delegate = new PaneDelegate(this);

    void expandItems_recursor(QStandardItem* item);
    void refresh();
    void addTempItem(QPoint eventPosition, Path::Type type);
    QStandardItem* tempItem(Path::Type type);
    const QString rename();

signals:
    bool askHasProject();
    void askTitleCheck();
    void askSendToEditor(QString key);
    void askDomMove(QString pivotKey, QString fulcrumKey, Io::Move position);
    void askAddElement(QString newName, Path::Type type, QString parentKey);
    void askRenameElement(QString newName, QString key);
    void askSetExpansion(QString key, bool isExpanded);
    void askCutElement(QString key);
};

// pane.h, fernanda