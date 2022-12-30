// path.h, Fernanda

#pragma once

#include <filesystem>
#include <string>
#include <type_traits>

#include <qsystemdetection.h>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>

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

#ifdef Q_OS_LINUX

	inline std::string toB7z(Fs::path path)
	{
		return path.string();
	}

#else

	inline std::string toB7z(Fs::path path)
	{
		auto result = toQString(path);
		result.replace(R"(/)", R"(\)");
		return result.toStdString();
	}

#endif

	inline void makeParent(Fs::path path)
	{
		auto parent = path.parent_path();
		if (QDir(parent).exists()) return;
		Fs::create_directories(parent);
	}

	template<typename T, typename U>
	inline const T getName(U path)
	{
		if constexpr (std::is_same<T, QString>::value && std::is_same<U, QString>::value)
			return QString::fromStdString(toFs(path).stem().string());
		if constexpr (std::is_same<T, QString>::value && std::is_same<U, Fs::path>::value)
			return QString::fromStdString(path.stem().string());
		if constexpr (std::is_same<T, Fs::path>::value && std::is_same<U, QString>::value)
			return toFs(path).stem();
		if constexpr (std::is_same<T, Fs::path>::value && std::is_same<U, Fs::path>::value)
			return path.stem();
	}

	inline void makeDirs(Fs::path dirPath)
	{
		if (QDir(dirPath).exists()) return;
		Fs::create_directories(dirPath);
	}
}

// path.h, Fernanda
