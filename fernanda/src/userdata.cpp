// userdata.cpp, fernanda

#include "userdata.h"

void Ud::windowsReg()
{
    //
}

void Ud::linuxReg()
{
    //
}

const QString Ud::userData(Op operation, QString name)
{
    if (name != nullptr)
        dataVars.appName = name;
    else
        name = dataVars.appName;
    auto user_data = (QDir::homePath() / QString("." + name));
    auto active_temp = user_data / ".active_temp";
    auto backup = user_data / "backup";
    auto dll = user_data / "dll";
    auto rollback = backup / ".rollback";
    auto config = user_data / name + ".ini";
    auto docs = QStandardPaths::locate(QStandardPaths::DocumentsLocation, nullptr, QStandardPaths::LocateDirectory);
    auto user_docs = docs / "Fernanda";
    QString result;
    switch (operation) {
    case Op::Config:
        result = config;
        break;
    case Op::Create:
        for (auto& data_folder : { user_data, active_temp, backup, dll, rollback, user_docs})
            Path::makeDirs(data_folder);
        result = nullptr;
        break;
    case Op::GetBackup:
        result = backup;
        break;
    case Op::GetDLL:
        result = dll;
        break;
    case Op::GetDocs:
        result = user_docs;
        break;
    case Op::GetRollback:
        result = rollback;
        break;
    case Op::GetTemp:
        result = active_temp;
        break;
    case Op::GetUserData:
        result = user_data;
        break;
    }
    return result;
}

void Ud::saveConfig(QString group, QString valueName, QVariant value)
{
    auto config = userData(Op::Config);
    QSettings ini(config, QSettings::IniFormat);
    ini.beginGroup(group);
    ini.setValue(valueName, value);
    ini.endGroup();
}

QVariant Ud::loadConfig(QString group, QString valueName, QVariant fallback, Ud::Type type)
{
    auto config = userData(Op::Config);
    if (!QFile(config).exists()) return fallback;
    QSettings ini(config, QSettings::IniFormat);
    if (!ini.childGroups().contains(group)) return fallback;
    ini.beginGroup(group);
    if (!ini.childKeys().contains(valueName)) return fallback;
    auto result = ini.value(valueName);
    if (type == Ud::Type::Bool) // no idea why, but switch is not working out here
        if (result != "true" && result != "false") return fallback;
    if (type == Ud::Type::Int)
        if (result.toInt() < 1) return fallback;
    if (type == Ud::Type::QRect)
        if (!result.canConvert<QRect>()) return fallback;
    ini.endGroup();
    return result;
}

void Ud::clear(QString dirPath, bool clearSelf)
{
    std::filesystem::path dir = dirPath.toStdString();
    for (auto& item : std::filesystem::directory_iterator(dir))
        std::filesystem::remove_all(item);
    if (clearSelf)
        std::filesystem::remove(dir);
}

int Ud::getTime()
{
    time_t now = time(0);
    return now;
}

std::wstring Ud::dll()
{
    auto dll_path = userData(Op::GetDLL) / "7z.dll";
    if (!Path::exists(dll_path))
        QFile::copy(":\\lib\\7zip 22.01\\7z64.dll", dll_path);
    return dll_path.toStdWString();
}

// userdata.cpp, fernanda