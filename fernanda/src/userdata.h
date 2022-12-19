// userdata.h, fernanda

#pragma once

#include "path.h"

#include <shlobj_core.h>

#include <filesystem>
#include <string>
#include <time.h>

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
    namespace Fs = std::filesystem;

    enum class ConfigGroup {
        Data,
        Editor,
        Window
    };

    enum class ConfigVal {
        Aot,
        BarAlign,
        CountChar,
        CountLine,
        CountWord,
        EditorTheme,
        Font,
        FontSize,
        PosCol,
        PosLine,
        Position,
        Project,
        Splitter,
        State,
        T_AotBtn,
        T_ColorBar,
        T_CursorBlink,
        T_Cursor,
        T_EditorTheme,
        T_Indicator,
        T_Keyfilter,
        T_Lmr,
        T_Lna,
        T_LineHighlight,
        T_Nav,
        T_Pane,
        T_Shadow,
        T_StatusBar,
        T_WinTheme,
        TabStop,
        WinTheme,
        Wrap
    };

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
        Bool,
        Int,
        QRect,
        QVariant
    };

    inline struct DataVars {
        QString appName;
    } dataVars;

    const QString editor = QStringLiteral("editor");
    const QString data = QStringLiteral("data");
    const QString window = QStringLiteral("window");

    void windowsReg(); // WIP
    void linuxReg(); // WIP
    void setName(QString name);
    const Fs::path userData(Op operation = Op::Create);
    void saveConfig(ConfigGroup group, ConfigVal valueType, QVariant value);
    QVariant loadConfig(ConfigGroup group, ConfigVal valueType, QVariant fallback = "", Ud::Type type = Ud::Type::QVariant);
    const QString groupName(ConfigGroup group);
    const QString valueName(ConfigVal valueType);
    void clear(Fs::path dirPath, bool clearSelf = false);
    QString timestamp();
    std::string dll();
}

// userdata.h, fernanda