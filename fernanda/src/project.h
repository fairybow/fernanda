// project.h, fernanda

#pragma once

//#include "microtar/microtar.h"

#include "io.h"
#include "sample.h"
#include "userdata.h"

#include <optional>
//#include <unordered_map>

#include <QStandardItem>
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
    ~Project() = default;

    QVector<QStandardItem*> items();
    const QString handleIO(QString newFileRelPath, QString oldText = "");
    QVector<QString> handleEditedList(QString currentText);
    QString getCurrentRelPath();
    void saveCurrent(QString text);
    void saveNonActiveFiles();
    bool hasEdits();
    // WIP below
    void createFile(std::filesystem::path relPath);
    void editFile(std::filesystem::path relPath, QString content);
    void deleteFile(std::filesystem::path relPath);
    void createFolder(std::filesystem::path relPath);
    void deleteFolder(std::filesystem::path relPath);
    void rename(std::filesystem::path relPath, std::string newName);
    void reparent(std::filesystem::path relPath, std::filesystem::path newPath);
    // ^ will move depth and will also pair files (aka reparent a file)
    void reorder(std::filesystem::path relPath, int nthChild);
    // end WIP

private:
    std::optional<QString> map;
    std::optional<QString> currentProjectName;
    std::optional<QString> currentRoot;
    std::optional<QString> currentRelFilePath;
    std::optional<QString> cleanText;
    QVector<QString> editedFiles;

    enum class PT {
        Main,
        Temp,
        Rollback
    };

    void tests(QString str = "");
    void makeProject(std::filesystem::path path, SP opt = SP::None);
    void makeXml(std::filesystem::path writePath, std::filesystem::path targetPath = "");
    void makeXml_recursor(QXmlStreamWriter& writer, std::filesystem::path readPath, std::filesystem::path rootPath = "");
    QString loadXml(std::filesystem::path path);
    void setProjectMap(QString xml);
    QStandardItem* items_recursor(QXmlStreamReader& reader);
    void tempSave(QString relFilePath, QString text);
    const QString handleNewFile(QString relFilePath);
    const QString derivePath(QString relFilePath, PT type);
};

// project.h, fernanda