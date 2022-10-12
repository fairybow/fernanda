// userdata.h, fernanda

#pragma once

#include <filesystem>
#include <string>
#include <time.h>

#include <QDir>
#include <QFile>
#include <QMetaType>
#include <QRect>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

namespace UD
{
    enum class Op {
        Config,
        Create,
        GetBackup,
        GetDocs,
        GetRollback,
        GetTemp,
        GetUserData
    };

    struct {
        QString appName;
    } dataVars;

    const QString userData(Op operation, QString name = "");
    void saveConfig(QString group, QString valueName, QVariant value);
    QVariant loadConfig(QString group, QString valueName, QVariant fallback = "", QMetaType::Type type = QMetaType::QVariant);
    void clearFiles(QString dirPath, bool clearSelf = false);
    int getTime();
    void makeDirs(QString dirPath);
}

// userdata.h, fernanda