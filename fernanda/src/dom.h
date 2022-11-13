// dom.h, fernanda

#pragma once

#include "io.h"

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>
#include <QVector>

class Dom
{

public:
    Dom() = default;
    ~Dom() = default;

    void set(QString xml);
    bool hasValue();
    const QString string();
    bool hasChanges();
    const QString relPath(QString key);
    const QString filteredRelPath(QString key);
    QVector<QDomElement> getElements(QString attribute, QString value = nullptr);
    const QString elementName(QString key);
    void moveElement(QString pivotKey, QString fulcrumKey, Io::Move pos); // clean me!
    void newElement(QString newName, Path::Type type, QString parentKey);
    void renameElement(QString newName, QString key);

private:
    struct MetaDocRename {
        QString key;
        QString changedRelPath;
        std::optional<QString> originalRelPath;
        bool operator==(const MetaDocRename&) const = default;
        bool operator!=(const MetaDocRename&) const = default;
    };

    QDomDocument self;
    QDomDocument initialSelf;
    QVector<MetaDocRename> renames_metaDoc;

    bool isChanged(QString key);
    QDomElement findElement(QString key);
    QDomElement findElement_recursor(QDomElement node, QString key, QDomElement result = QDomElement());
    QVector<QDomElement> getElements_recursor(QDomElement node, QString attribute, QString value = nullptr, QVector<QDomElement> result = QVector<QDomElement>());
    QString nearestParentKeyOfType_recursor(QDomElement node, Path::Type type, QString result = nullptr);
    QVector<MetaDocRename> getElementChildTree_recursor(QDomElement node, QString stemPathParent, QVector<MetaDocRename> result = QVector<MetaDocRename>());
    void handleRenamesList(QVector<MetaDocRename> renames);
    void makeMovePaths(QString& newPivotPath, QString& newPivotParentPath, QString pivotName, QDomElement fulcrumNode); // move to Path?
};

// dom.h, fernanda