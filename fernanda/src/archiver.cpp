// archiver.cpp, fernanda

#include "archiver.h"

using namespace bit7z;

void Archiver::create(QString arcPath, QVector<Io::ArcWRPaths> wRPaths)
{
	QTemporaryDir temp_dir;
	auto temp_dir_path = temp_dir.path();
	for (auto& entry : wRPaths)
	{
		auto temp_w_path = (temp_dir_path / entry.writeRelPath).toStdString();
		if (!entry.readFullPath.has_value())
			Path::makeDirs(temp_w_path);
		else
		{
			Path::makeDirs(std::filesystem::path(temp_w_path).parent_path());
			auto q_path = QString::fromStdString(temp_w_path);
			QFile::copy(entry.readFullPath.value(), q_path);
			QFile(q_path).setPermissions(QFile::WriteUser);
		}
	}
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.compressDirectory((temp_dir_path / "story").toStdString(), arcPath.toStdString());
}

const QString Archiver::read(QString arcPath, QString rPath)
{
	QString result = nullptr;
	QTemporaryDir temp_dir;
	auto temp_dir_path = temp_dir.path();
	auto was_found = extractMatch(arcPath, rPath, temp_dir_path);
	if (was_found)
		result = Io::readFile(temp_dir_path / rPath);
	return result;
}

bool Archiver::extractMatch(QString arcPath, QString relPath, QString extractPath)
{
	try {
		Bit7zLibrary lib{ Ud::dll() };
		BitFileExtractor extractor{ lib, format };
		extractor.extractMatching(arcPath.toStdString(), relPath.toStdString(), extractPath.toStdString());
	}
	catch (const BitException&) {
		return false;
	}
	return true;
}

void Archiver::extract(QString arcPath, QString exPath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileExtractor extractor{ lib, format };
	extractor.extract(arcPath.toStdString(), exPath.toStdString());
}

void Archiver::add(QString arcPath, QString rPath, QString wPath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(UpdateMode::Overwrite);
	std::map<std::string, std::string> in_map;
	in_map[rPath.toStdString()] = wPath.toStdString();
	compressor.compress(in_map, arcPath.toStdString());
}

void Archiver::add(QString arcPath, QVector<Io::ArcWRPaths> wRPaths)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(UpdateMode::Overwrite);
	std::map<std::string, std::string> in_map;
	for (auto& wr_path : wRPaths)
		in_map[wr_path.readFullPath.value().toStdString()] = wr_path.writeRelPath.toStdString();
	compressor.compress(in_map, arcPath.toStdString());
}

void Archiver::add(QString arcPath, Io::ArcWrite textAndWPath)
{
	QTemporaryDir temp_dir;
	auto& w_path = textAndWPath.writeRelPath;
	auto temp_path = temp_dir.path() / Path::getName(w_path);
	Io::writeFile(temp_path, textAndWPath.text);
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(UpdateMode::Overwrite);
	std::map<std::string, std::string> in_map;
	in_map[temp_path.toStdString()] = w_path.toStdString();
	compressor.compress(in_map, arcPath.toStdString());
}

void Archiver::save(QString arcPath, QVector<Io::ArcRename> renamePaths)
{
	std::map<std::string, Path::Type> additions;
	std::map<std::string, std::string> renames;
	for (auto& entry : renamePaths)
	{
		if (entry.typeIfNewOrCut.has_value())
			additions[entry.relPath.toStdString()] = entry.typeIfNewOrCut.value();
		else
			renames[entry.relPath.toStdString()] = entry.origRelPath.value().toStdString();
	}
	if (!renames.empty())
		rename(arcPath, renames);
	if (!additions.empty())
		blanks(arcPath, additions);
}

void Archiver::cut(QString arcPath, QVector<Io::ArcRename> cuts)
{
	std::map<std::string, std::string> in_map;
	std::vector<std::string> cut_folders;
	for (auto& cut : cuts)
	{
		if (!cut.origRelPath.has_value()) continue;
		if (cut.typeIfNewOrCut != Path::Type::Dir)
			in_map[QString(".cut" / Path::getName(cut.relPath) + ".txt~").toStdString()] = cut.relPath.toStdString();
		else
			cut_folders << cut.origRelPath.value().toStdString();
	}
	rename(arcPath, in_map);
	if (!cut_folders.empty())
		del(arcPath, cut_folders);
}

void Archiver::rename(QString arcPath, std::map<std::string, std::string> renames)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitArchiveEditor editor{ lib, arcPath.toStdString(), format };
	for (const auto& [key, value] : renames)
		editor.renameItem(value, key);
	editor.applyChanges();
}

void Archiver::del(QString arcPath, std::vector<std::string> relPaths)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitArchiveEditor editor{ lib, arcPath.toStdString(), format };
	for (const auto& rel_path : relPaths)
		editor.deleteItem(rel_path);
	editor.applyChanges();
}

void Archiver::blanks(QString arcPath, std::map<std::string, Path::Type> additions)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(UpdateMode::Overwrite);
	QTemporaryDir temp_dir;
	auto temp_dir_path = temp_dir.path();
	for (const auto& [key, value] : additions)
	{
		auto temp_r_path = std::filesystem::path(temp_dir_path.toStdString()) / std::filesystem::path(key);
		if (value == Path::Type::Dir)
			Path::makeDirs(temp_r_path);
		else
		{
			Path::makeDirs(std::filesystem::path(temp_r_path).parent_path());
			Io::writeFile(QString::fromStdString(temp_r_path.string()), nullptr);
		}
	}
	compressor.compressDirectory((temp_dir_path / "story").toStdString(), arcPath.toStdString());
}

// archiver.cpp, fernanda