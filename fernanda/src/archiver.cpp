// archiver.cpp, fernanda

#include "archiver.h"

using namespace bit7z;

void Archiver::create(QString readPath, QString writePath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.compressDirectory(readPath.toStdWString(), writePath.toStdWString());
}

void Archiver::add(std::map<std::wstring, std::wstring> inMap, QString filePath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitCompressor compressor{ lib, format };
	compressor.setCompressionLevel(level);
	compressor.setUpdateMode(true);
	compressor.compress(inMap, filePath.toStdWString());
}

void Archiver::extract(QString filePath, QString extractPath)
{
	Bit7zLibrary lib{ Ud::dll() };
	BitExtractor extractor{ lib, format };
	extractor.extract(filePath.toStdWString(), extractPath.toStdWString());
}

bool Archiver::extractMatch(QString filePath, QString relPath, QString extractPath)
{
	try {
		Bit7zLibrary lib{ Ud::dll() };
		BitExtractor extractor{ lib, format };
		extractor.extractMatching(filePath.toStdWString(), relPath.toStdWString(), extractPath.toStdWString());
	}
	catch (const BitException&) {
		return false;
	}
	return true;
}

// archiver.cpp, fernanda