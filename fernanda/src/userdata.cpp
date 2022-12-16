// userdata.cpp, fernanda

#include "userdata.h"

void Ud::windowsReg() // WIP
{
    //QCoreApplication::setOrganizationName("@fairybow");
    //QCoreApplication::setApplicationName("fernanda");
    //QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\.story\\DefaultIcon", QSettings::NativeFormat);
    //reg.setValue("DefaultIcon", "C:\\Dev\\fernanda-dev\\fernanda\\res\\icons\\fernanda.ico");
    //reg.setValue("DefaultIcon", ":\\icons\\fernanda.ico");
    //SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void Ud::linuxReg() // WIP
{
    //
}

void Ud::setName(QString name)
{
    dataVars.appName = name;
}

const QString Ud::userData(Op operation)
{
    auto user_data = (QDir::homePath() / QString("." + dataVars.appName));
    auto active_temp = user_data / ".active_temp";
    auto backup = user_data / "backup";
    auto dll = user_data / "dll";
    auto rollback = backup / ".rollback";
    auto config = user_data / dataVars.appName + ".ini";
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
    auto bad_result = false;
    switch (type) {
    case Ud::Type::Bool:
        if (result != "true" && result != "false")
            bad_result = true;
        break;
    case Ud::Type::Int:
        if (result.toInt() < 1)
            bad_result = true;
        break;
    case Ud::Type::QRect:
        if (!result.canConvert<QRect>())
            bad_result = true;
        break;
    default: break;
    }
    ini.endGroup();
    if (bad_result) return fallback;
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

QString Ud::timestamp()
{
    const time_t now = std::time(0);
    return QString::fromLocal8Bit(std::ctime(&now));
}

std::string Ud::dll()
{
    auto dll_path = userData(Op::GetDLL) / "7z.dll";
    if (!QFile(dll_path).exists())
        QFile::copy(":\\lib\\7zip\\7z64.dll", dll_path);
    return dll_path.toStdString();
}

// userdata.cpp, fernanda