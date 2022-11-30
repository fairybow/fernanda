// project.h, fernanda

#pragma once

#include "archiver.h"
#include "dom.h"
#include "sample.h"
#include "uni.h"

#include <QByteArray>
#include <QStandardItem>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Project
{

public:
    enum class SP {
        None = 0,
        MakeSample
    };

    Project(QString filePath, SP opt = SP::None);

    QVector<QStandardItem*> makeItems();
    const QString getActiveKey();
    void autoTempSave(QString oldText);
    const QString saveOld_openNew(QString newKey, QString oldText = nullptr);
    QVector<QString> handleEditsList(QString currentText);
    bool hasAnyChanges();
    void saveProject(QString currentText); // WIP
    void domMove(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void newDomElement(QString newName, Path::Type type, QString parentKey);
    void renameDomElement(QString newName, QString key);
    void setDomElementExpansionState(QString key, bool isExpanded);

private:
    Archiver* archiver = new Archiver;
    Dom* dom = new Dom;
    QString activeArchive = nullptr;
    QString activeKey = nullptr;
    QString cleanText = nullptr;
    QVector<QString> edits_metaDoc;

    void makeProject(QString filePath, SP opt = SP::None);
    void createArc(QVector<Io::ArchivePaths> dataList, QString writePath);
    void addToArc(QVector<Io::ArchivePaths> dataList, QString filePath);
    const QString loadXml(QString filePath);
    void newXml(QString filePath);
    void newXml_recursor(QXmlStreamWriter& writer, QString readPath, QString rootPath = nullptr);
    void extractArc(QString filePath, QString extractPath);
    QStandardItem* makeItems_recursor(QXmlStreamReader& reader);
    void tempSave(QString key, QString text);
    const QString tempOpen(QString newKey);
    const QString readInArchive(QString filePath, QString relPath);
    const QString tempPath(QString key);
    bool isEdited(QString key);
    void bak();
};

// project.h, fernanda