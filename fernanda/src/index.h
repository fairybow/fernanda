// index.h, fernanda

#pragma once

#include <QModelIndex>
#include <QString>
#include <Qt>
#include <QVariant>

namespace Index
{
	inline const QString type(QModelIndex index)
	{
		if (!index.isValid()) return nullptr;
		return index.data(Qt::UserRole).toString();
	}

	inline const QString name(QModelIndex index)
	{
		if (!index.isValid()) return nullptr;
		return index.data(Qt::UserRole + 1).toString();
	}

	inline const QString key(QModelIndex index)
	{
		if (!index.isValid()) return nullptr;
		return index.data(Qt::UserRole + 2).toString();
	}

	inline bool isExpanded(QModelIndex index)
	{
		if (!index.isValid()) return false;
		return index.data(Qt::UserRole + 3).toBool();
	}

	inline bool isDir(QModelIndex index)
	{
		if (type(index) == "dir") return true;
		return false;
	}

	inline bool isFile(QModelIndex index)
	{
		if (type(index) == "file") return true;
		return false;
	}
}

// index.h, fernanda