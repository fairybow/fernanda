// archiver.h, fernanda

#pragma once

#include "bit7z/include/bit7z.hpp"
#include "bit7z/include/bitarchiveeditor.hpp"

#include "io.h"
#include "userdata.h"

#include <map>
#include <vector>

#include <QTemporaryDir>

inline void operator<<(std::vector<std::string>& lhs, const std::string& rhs)
{
    return lhs.push_back(rhs);
}

class Archiver
{
    using FsPath = std::filesystem::path;

public:
    void create(FsPath arcPath, QVector<Io::ArcWRPaths> wRPaths);
    const QString read(FsPath arcPath, FsPath rPath);
    bool extractMatch(FsPath arcPath, FsPath relPath, FsPath exPath);
    void extract(FsPath arcPath, FsPath exPath);
    void add(FsPath arcPath, FsPath rPath, FsPath wPath);
    void add(FsPath arcPath, QVector<Io::ArcWRPaths> wRPaths);
    void add(FsPath arcPath, Io::ArcWrite textAndWPath);
    void save(FsPath arcPath, QVector<Io::ArcRename> renamePaths);
    void cut(FsPath arcPath, QVector<Io::ArcRename> cuts);

private:
    const bit7z::BitInOutFormat& format = bit7z::BitFormat::SevenZip;
    bit7z::BitCompressionLevel level = bit7z::BitCompressionLevel::None;

    void rename(FsPath arcPath, std::map<std::string, std::string> renames);
    void del(FsPath arcPath, std::vector<std::string> relPaths);
    void blanks(FsPath arcPath, std::map<std::string, Path::Type> additions);
};

// archiver.h, fernanda