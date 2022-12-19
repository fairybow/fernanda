// path.h, fernanda

#pragma once

#include <filesystem>
#include <string>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>

inline void operator<<(std::vector<std::string>& lhs, const std::string& rhs)
{
	return lhs.push_back(rhs);
}

namespace Path
{
	namespace Fs = std::filesystem;

	enum class Type {
		Dir,
		File
	};

	inline Fs::path toFs(QString qStringPath)
	{
		return Fs::path(qStringPath.toStdString());
	}

	inline Fs::path toFs(QVariant qVariantPath)
	{
		return Fs::path(qVariantPath.toString().toStdString());
	}

	inline QString toQString(Fs::path path, bool sanitize = false)
	{
		auto result = QString::fromStdString(path.make_preferred().string());
		if (sanitize)
			result.replace(R"(\)", R"(/)");
		return result;
	}

	inline std::string toB7z(Fs::path path)
	{
		auto result = toQString(path);
		result.replace(R"(/)", R"(\)");
		return result.toStdString();
	}

	inline void makeParent(Fs::path path)
	{
		auto parent = path.parent_path();
		if (QDir(parent).exists()) return;
		Fs::create_directories(parent);
	}

	inline const QString getName(Fs::path path)
	{
		return QString::fromStdString(path.stem().string());
	}

	inline void makeDirs(Fs::path dirPath)
	{
		if (QDir(dirPath).exists()) return;
		Fs::create_directories(dirPath);
	}
}

// path.h, fernanda