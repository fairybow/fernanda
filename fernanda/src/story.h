// story.h, fernanda

#pragma once

#include "archiver.h"
#include "dom.h"
#include "sample.h"
#include "uni.h"

#include <QStandardItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Story
{

public:
    enum class Op {
        Normal,
        Sample
    };

    Story(QString filePath, Op opt = Op::Normal);

    const QString devGetDom(Dom::Doc doc = Dom::Doc::Current);
    QVector<Io::ArcRename> devGetRenames();
    QVector<QStandardItem*> items();
    const QString key();
    const QString tempSaveOld_openNew(QString newKey, QString oldText = nullptr);
    void autoTempSave(QString text);
    QVector<QString> edits(QString currentText);
    bool hasChanges();
    void setItemExpansion(QString key, bool isExpanded);
    void move(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void rename(QString newName, QString key);
    void add(QString newName, Path::Type type, QString parentKey);
    void cut(QString key);
    void save(QString text = nullptr);

private:
    Archiver* archiver = new Archiver;
    Dom* dom = new Dom;

    QString activeArcPath = nullptr;
    QString activeKey = nullptr;
    QString cleanText = nullptr;
    QVector<QString> editedKeys;

    enum class AmendEdits {
        Add,
        Remove
    };

    void make(Op opt);
    const QString xml();
    void newXml();
    void newXml_recursor(QXmlStreamWriter& writer, QString rPath, QString rootPath = nullptr);
    QStandardItem* items_recursor(QXmlStreamReader& reader);
    void tempSave(QString key, QString text);
    const QString tempOpen(QString newKey);
    const QString tempPath(QString key);
    void amendEditsList(AmendEdits op);
    bool isEdited(QString key);
    void bak();
};

// story.h, fernanda