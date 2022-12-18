// userdata.cpp, fernanda

#include "userdata.h"

#include <filesystem>
#include <stdexcept>

void Ud::windowsReg() // WIP
{
    //
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
	const auto paths = QStringList{"/lib", "/usr/lib", "/usr/local/lib" }<<qEnvironmentVariable("LD_LIBRARY_PATH").split(',');

	for(const auto& search_path: paths)
	{
		for(const auto& libname: {"7z.so", "p7zip/7z.so"})
		{
			const auto candidate = std::filesystem::path{search_path.toStdString()}/libname;
			if(std::filesystem::exists(candidate))
				return candidate;
		}
	}
	throw std::runtime_error("Unable to locate shared 7z library, did you install all dependencies?");
}

// userdata.cpp, fernanda
