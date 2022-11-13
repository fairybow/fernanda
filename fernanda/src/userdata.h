// userdata.h, fernanda

#pragma once

#include <shlobj_core.h>

#include <filesystem>
#include <string>
#include <time.h>

#include "path.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRect>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

namespace Ud
{
    enum class Op {
        Config,
        Create,
        GetBackup,
        GetDLL,
        GetDocs,
        GetRollback,
        GetTemp,
        GetUserData
    };

    enum class Type {
        QVariant = 0,
        Bool,
        Int,
        QRect
    };

    struct {
        QString appName;
    } dataVars;

    void windowsReg();
    void linuxReg();
    const QString userData(Op operation, QString name = nullptr);
    void saveConfig(QString group, QString valueName, QVariant value);
    QVariant loadConfig(QString group, QString valueName, QVariant fallback = "", Ud::Type type = Ud::Type::QVariant);
    void clear(QString dirPath, bool clearSelf = false);
    int getTime();
    std::wstring dll();
}

// userdata.h, fernanda