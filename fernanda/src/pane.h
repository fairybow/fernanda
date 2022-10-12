// pane.h, fernanda

#pragma once

#include "delegate.h"

#include <QAbstractItemView>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>

class Pane : public QTreeView
{
    Q_OBJECT

public:
    Pane(QWidget* parent = nullptr);
    ~Pane() = default;

public slots:
    void receiveItems(QVector<QStandardItem*> itemsVector);
    void receiveEditsList(QVector<QString> editedFiles);
    void replyNavPrevious();
    void replyNavNext();

private:
    QStandardItemModel* itemModel = new QStandardItemModel(this);
    PaneDelegate* paneDelegate = new PaneDelegate(this);
    enum class Nav {
        Prev,
        Next
    };
    void contextMenuEvent(QContextMenuEvent* event) override;
    void refresh();
    void nav(Nav direction);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
    bool askHasProject();
    void askSendToEditor(QString relFilePath);
};

// pane.h, fernanda