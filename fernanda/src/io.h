// io.h, fernanda

#pragma once

#include "path.h"

#include <optional>

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

	enum class AmendVector {
		Add,
		Remove
	};

	struct ArchivePaths {
		QString archivePath;
		std::optional<QString> readPath;
		//bool operator==(const ArchivePaths&) const = default;
		//bool operator!=(const ArchivePaths&) const = default;
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

	inline void writeFile(QString filePath, QString text)
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

	template<typename T>
	inline void amendVector(QVector<T>& vector, T item, AmendVector op)
	{
		switch (op) {
		case AmendVector::Add:
			if (!vector.contains(item))
				vector << item;
			break;
		case AmendVector::Remove:
			if (vector.contains(item))
				vector.removeAll(item);
			break;
		}
	}
}

// io.h, fernanda