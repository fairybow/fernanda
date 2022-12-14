// archiver.h, fernanda

#pragma once

#include "bit7z/include/bit7z.hpp"
#include "bit7z/include/bitarchiveeditor.hpp"

#include "io.h"
#include "userdata.h"

#include <map>
#include <vector>

#include <QTemporaryDir>

class Archiver
{

public:
    void create(QString arcPath, QVector<Io::ArcWRPaths> wRPaths);
    const QString read(QString arcPath, QString rPath);
    bool extractMatch(QString arcPath, QString relPath, QString exPath);
    void extract(QString arcPath, QString exPath);
    void add(QString arcPath, QString rPath, QString wPath);
    void add(QString arcPath, QVector<Io::ArcWRPaths> wRPaths);
    void add(QString arcPath, Io::ArcWrite textAndWPath);
    void save(QString arcPath, QVector<Io::ArcRename> renamePaths);
    void cut(QString arcPath, QVector<Io::ArcRename> cuts);

private:
    const bit7z::BitInOutFormat& format = bit7z::BitFormat::SevenZip;
    bit7z::BitCompressionLevel level = bit7z::BitCompressionLevel::None;

    void rename(QString arcPath, std::map<std::string, std::string> renames);
    void del(QString arcPath, std::vector<std::string> relPaths);
    void blanks(QString arcPath, std::map<std::string, Path::Type> additions);
};

// archiver.h, fernanda