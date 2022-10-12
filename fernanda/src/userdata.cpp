// userdata.cpp, fernanda

#include "userdata.h"

const QString UD::userData(Op operation, QString name)
{
    if (!name.isEmpty())
        dataVars.appName = name;
    else
        name = dataVars.appName;
    auto user_data = std::filesystem::path(QDir::homePath().toStdString()) / std::string("." + name.toStdString());
    auto active_temp = user_data / std::string(".active_temp");
    auto backup = user_data / std::string("backup");
    auto rollback = backup / std::string(".rollback");
    auto config = user_data / std::string(name.toStdString() + ".ini");
    std::filesystem::path docs = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory).toStdString();
    auto user_docs = docs / std::string("Fernanda");
    QString result;
    switch (operation) {
        case Op::Config:
            result = QString::fromStdString(config.string());
            break;
        case Op::Create:
            for (auto& data_folder : { user_data, active_temp, backup, rollback, user_docs })
                UD::makeDirs(QString::fromStdString(data_folder.string()));
            result = nullptr;
            break;
        case Op::GetBackup:
            result = QString::fromStdString(backup.string());
            break;
        case Op::GetDocs:
            result = QString::fromStdString(user_docs.string());
            break;
        case Op::GetRollback:
            result = QString::fromStdString(rollback.string());
            break;
        case Op::GetTemp:
            result = QString::fromStdString(active_temp.string());
            break;
        case Op::GetUserData:
            result = QString::fromStdString(user_data.string());
            break;
    }
    return result;
}

void UD::saveConfig(QString group, QString valueName, QVariant value)
{
    auto config = userData(Op::Config);
    QSettings ini(config, QSettings::IniFormat);
    ini.beginGroup(group);
    ini.setValue(valueName, value);
    ini.endGroup();
}

QVariant UD::loadConfig(QString group, QString valueName, QVariant fallback, QMetaType::Type type)
{
    auto config = userData(Op::Config);
    if (!QFile(config).exists()) return fallback;
    QSettings ini(config, QSettings::IniFormat);
    if (!ini.childGroups().contains(group)) return fallback;
    ini.beginGroup(group);
    if (!ini.childKeys().contains(valueName)) return fallback;
    auto result = ini.value(valueName);
    if (type == QMetaType::Bool)
        if (result != "true" && result != "false") return fallback;
    if (type == QMetaType::Int)
        if (result.toInt() < 1) return fallback;
    if (type == QMetaType::QRect)
        if (!result.canConvert<QRect>()) return fallback;
    ini.endGroup();
    return result;
}

void UD::clearFiles(QString dirPath, bool clearSelf)
{
    std::filesystem::path dir = dirPath.toStdString();
    for (auto& item : std::filesystem::directory_iterator(dir))
        std::filesystem::remove_all(item);
    if (clearSelf)
        std::filesystem::remove(dir);
}

int UD::getTime()
{
    time_t now = time(0);
    return now;
}

void UD::makeDirs(QString dirPath)
{
    if (QDir(dirPath).exists()) return;
    std::filesystem::path new_path = dirPath.toStdString();
    std::filesystem::create_directories(new_path);
}

// userdata.cpp, fernanda