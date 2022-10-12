// io.h, fernanda

#pragma once

#include <filesystem>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QTextStream>

namespace IO
{
	inline const QString readFile(QString filePath)
	{
		QString text;
		QFile file(filePath);
		if (file.open(QFile::ReadOnly | QIODevice::Text))
		{
			QTextStream in(&file);
			text = in.readAll();
			file.close();
		}
		return text;
	}

	inline void writeFile(QString filePath, QString text)
	{
		QFile file(filePath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream out(&file);
			out << text;
			file.close();
		}
	}

	inline bool checkExists(QString path)
	{
		if (QFileInfo(path).isFile())
		{
			if (QFile(path).exists()) return true;
		}
		else
			if (QDir(path).exists()) return true;
		return false;
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
			tmp = QString(path + "//").toStdString();
			name = tmp.parent_path().stem();
		}
		return QString::fromStdString(name.string());
	}

	inline QString relPath(std::filesystem::path rootPath, std::filesystem::path currentPath)
	{
		auto rel = relative(currentPath, rootPath);
		return QString::fromStdString(rel.make_preferred().string());
	}
}

// io.h, fernanda