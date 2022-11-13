// sample.h, fernanda

#pragma once

#include "io.h"

#include <filesystem>
#include <string>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QVector>

namespace Sample
{
	struct SampleRCPair {
		std::filesystem::path rcPath;
		std::filesystem::path udPath;
	};

	inline QVector<Io::ArchivePaths> make()
	{
		QVector<Io::ArchivePaths> result;
		auto rootPath = ":\\sample\\Candide\\";
		QDirIterator it(rootPath, QVector<QString>() << "*.*", QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			it.next();
			auto read_path = it.filePath();
			auto rel_path = Path::relPath(rootPath, read_path);
			if (it.fileInfo().isDir())
				result << Io::ArchivePaths{ "story" / rel_path };
			else
				result << Io::ArchivePaths{ "story" / rel_path, read_path };
		}
		return result;
	}

	inline void makeRc(std::filesystem::path path)
	{
		QVector<SampleRCPair> pairs{
			SampleRCPair{ ":\\sample\\Merriweather.ttf", std::filesystem::path(path / "Merriweather.ttf") },
			SampleRCPair{ ":\\sample\\sample.fernanda_theme", std::filesystem::path(path / "sample.fernanda_theme") },
			SampleRCPair{ ":\\sample\\sample.fernanda_wintheme", std::filesystem::path(path / "sample.fernanda_wintheme") }
		};
		for (auto& pair : pairs)
		{
			auto lhs = QString::fromStdString(pair.rcPath.string());
			auto rhs = QString::fromStdString(pair.udPath.string());
			if (QFile(rhs).exists())
				QFile(rhs).moveToTrash(); // font will be held by system
			QFile::copy(lhs, rhs);
			QFile(rhs).setPermissions(QFile::WriteUser);
		}
	}
}

// sample.h, fernanda