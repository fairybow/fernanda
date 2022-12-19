// io.h, fernanda

#pragma once

#include "path.h"

#include <optional>
#include <utility>

#include <QFile>
#include <QIODevice>
#include <QTextStream>

namespace Io
{
	namespace Fs = std::filesystem;

	enum class Move {
		Above,
		Below,
		On,
		Viewport
	};

	struct ArcWRPaths {
		Fs::path writeRelPath;
		std::optional<Fs::path> readFullPath;
	};
	struct ArcWrite {
		QString text;
		Fs::path writeRelPath;
	};
	struct ArcRename {
		QString key;
		Fs::path relPath;
		std::optional<Fs::path> origRelPath;
		std::optional<Path::Type> typeIfNewOrCut;
	};

	const Fs::path storyRoot = QStringLiteral("story").toStdString();
	const QString ext = QStringLiteral(".txt");
	const QString tempExt = QStringLiteral(".txt~");

	inline const QString readFile(Fs::path filePath)
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

	inline void writeFile(Fs::path filePath, QString text)
	{
		Path::makeParent(filePath);
		QFile file(filePath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream out(&file);
			out << text;
			file.close();
		}
	}
}

// io.h, fernanda