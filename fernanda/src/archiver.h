// archiver.h, fernanda

#pragma once

#include "bit7z/include/bit7z.hpp"

#include "userdata.h"

#include <map>
#include <type_traits>

class Archiver
{

public:
    enum class Op {
        Add,
        Create,
        Extract
    };

    template<typename T, typename U>
    void arc(Op operation, T in, U out)
    {
        switch (operation) {
        case Op::Add:
            if constexpr (std::is_same<T, std::map<std::wstring, std::wstring>>::value)
                add(in, out);
            break;
        case Op::Create:
            if constexpr (std::is_same<T, QString>::value)
                create(in, out);
            break;
        case Op::Extract:
            if constexpr (std::is_same<T, QString>::value)
                extract(in, out);
            break;
        }
    }

    template<typename T, typename U, typename V>
    bool arc(Op operation, T in, U filter, V out)
    {
        if (operation != Op::Extract) return false;
        auto was_found = extractMatch(in, filter, out);
        return was_found;
    }

private:
    const bit7z::BitInOutFormat& format = bit7z::BitFormat::SevenZip;
    bit7z::BitCompressionLevel level = bit7z::BitCompressionLevel::NONE;

    void create(QString readPath, QString writePath);
    void add(std::map<std::wstring, std::wstring> inMap, QString filePath);
    void extract(QString filePath, QString extractPath);
    bool extractMatch(QString filePath, QString relPath, QString extractPath);
};

// archiver.h, fernanda