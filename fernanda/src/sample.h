// sample.h, fernanda

#pragma once

#include <filesystem>
#include <string>
#include <tuple>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QVector>

namespace Sample
{
	inline void make(std::filesystem::path path)
	{
		QDirIterator files_1(":/sample/Candide/Chapters 1-10/", QStringList() << "*.txt", QDir::Files);
		QDirIterator files_2(":/sample/Candide/Chapters 11-20/", QStringList() << "*.txt", QDir::Files);
		QDirIterator files_3(":/sample/Candide/Chapters 21-30/", QStringList() << "*.txt", QDir::Files);
		QDirIterator files_4(":/sample/Candide/Misc/", QStringList() << "*.txt", QDir::Files);
		auto subfolder_1 = path / std::string("Chapters 1-10");
		auto subfolder_2 = path / std::string("Chapters 11-20");
		auto subfolder_3 = path / std::string("Chapters 21-30");
		auto subfolder_4 = path / std::string("Misc");
		for (auto& folder : { subfolder_1, subfolder_2, subfolder_3, subfolder_4 })
			std::filesystem::create_directory(folder);
		QVector<std::tuple<QDirIterator&, std::filesystem::path>> iteratorAndPathPairs = {
			std::tuple<QDirIterator&, std::filesystem::path>(files_1, subfolder_1),
			std::tuple<QDirIterator&, std::filesystem::path>(files_2, subfolder_2),
			std::tuple<QDirIterator&, std::filesystem::path>(files_3, subfolder_3),
			std::tuple<QDirIterator&, std::filesystem::path>(files_4, subfolder_4),
		};
		for (auto& pair : iteratorAndPathPairs)
		{
			auto& iterator = std::get<0>(pair);
			auto& folder_path = std::get<1>(pair);
			while (iterator.hasNext())
			{
				iterator.next();
				auto file = iterator.filePath();
				auto name = iterator.fileName();
				auto path = folder_path / name.toStdString();
				auto copy_path = QString::fromStdString(path.string());
				QFile::copy(file, copy_path);
				QFile(copy_path).setPermissions(QFile::WriteUser);
			}
		}
	}

	inline void makeRc(std::filesystem::path path)
	{
		auto font_fs = path / "Merriweather.ttf";
		auto editor_theme_fs = path / "sample.fernanda_theme";
		auto win_theme_fs = path / "sample.fernanda_wintheme";
		QVector<std::tuple<QString, QString>> file_pairs = {
			std::tuple<QString, QString>(":/sample/Merriweather.ttf", QString::fromStdString(font_fs.string())),
			std::tuple<QString, QString>(":/sample/sample.fernanda_theme", QString::fromStdString(editor_theme_fs.string())),
			std::tuple<QString, QString>(":/sample/sample.fernanda_wintheme", QString::fromStdString(win_theme_fs.string()))
		};
		for (auto& file_pair : file_pairs)
		{
			auto& lhs = std::get<0>(file_pair);
			auto& rhs = std::get<1>(file_pair);
			if (QFile(rhs).exists())
				QFile(rhs).moveToTrash(); // font will be held by system
			QFile::copy(lhs, rhs);
			QFile(rhs).setPermissions(QFile::WriteUser);
		}
	}
}

// sample.h, fernanda