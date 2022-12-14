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
	enum class Move {
		Above,
		Below,
		On,
		Viewport
	};

	struct ArcWRPaths {
		QString writeRelPath;
		std::optional<QString> readFullPath;
	};
	struct ArcWrite {
		QString text;
		QString writeRelPath;
	};
	struct ArcRename {
		QString key;
		QString relPath;
		std::optional<QString> origRelPath;
		std::optional<Path::Type> typeIfNewOrCut;
	};

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

	inline void writeFile(QString filePath, QString text = nullptr)
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