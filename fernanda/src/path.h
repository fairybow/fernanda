// path.h, fernanda

#pragma once

#include <filesystem>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>

inline QString operator/(const QString& lhs, const QString& rhs)
{
	return lhs + "\\" + rhs;
}

namespace Path
{
	enum class Type {
		Dir,
		File
	};

	inline void makeParent(QString path)
	{
		std::filesystem::path tmp = path.toStdString();
		auto parent = tmp.parent_path();
		if (QDir(parent).exists()) return;
		std::filesystem::create_directories(parent);
	}

	inline void makeParent(std::filesystem::path path)
	{
		auto parent = path.parent_path();
		if (QDir(parent).exists()) return;
		std::filesystem::create_directories(parent);
	}

	inline QString getName(QString path)
	{
		std::filesystem::path name;
		std::filesystem::path tmp;
		if (QFileInfo(path).isFile())
		{
			tmp = path.toStdString();
			name = tmp.stem();
		}
		else
		{
			tmp = QString(path + "\\").toStdString();
			name = tmp.parent_path().stem();
		}
		return QString::fromStdString(name.string());
	}

	inline QString makePreferred(QString path)
	{
		return QString::fromStdString(std::filesystem::path(path.toStdString()).make_preferred().string());
	}

	inline QString makePosix(QString path)
	{
		return path.replace(QRegularExpression(R"(\)"), "/");
	}

	inline QString relPath(QString rootPath, QString currentPath)
	{
		auto rel = relative(std::filesystem::path(currentPath.toStdString()), std::filesystem::path(rootPath.toStdString()));
		return makePreferred(QString::fromStdString(rel.string()));
	}

	inline void makeDirs(QString dirPath)
	{
		if (QDir(dirPath).exists()) return;
		std::filesystem::path new_path = dirPath.toStdString();
		std::filesystem::create_directories(new_path);
	}

	inline void makeDirs(std::filesystem::path dirPath)
	{
		if (QDir(dirPath).exists()) return;
		std::filesystem::create_directories(dirPath);
	}
}

// path.h, fernanda