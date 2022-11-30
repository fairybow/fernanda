// archiver.cpp, fernanda

#include "archiver.h"

using namespace bit7z;

void Archiver::create(QString readPath, QString writePath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.compressDirectory(readPath.toStdString(), writePath.toStdString());
}

void Archiver::add(std::map<std::string, std::string> inMap, QString filePath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(UpdateMode::Overwrite);
	compressor.compress(inMap, filePath.toStdString());
}

void Archiver::extract(QString filePath, QString extractPath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitFileExtractor extractor{ lib, format };
	extractor.extract(filePath.toStdString(), extractPath.toStdString());
}

bool Archiver::extractMatch(QString filePath, QString relPath, QString extractPath)
{
	try {
		Bit7zLibrary lib{ Ud::dll() };
		BitFileExtractor extractor{ lib, format };
		extractor.extractMatching(filePath.toStdString(), relPath.toStdString(), extractPath.toStdString());
	}
	catch (const BitException&) {
		return false;
	}
	return true;
}

// archiver.cpp, fernanda