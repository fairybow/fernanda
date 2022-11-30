// res.h, fernanda

#pragma once

#include "uni.h"

#include <algorithm>
#include <filesystem>
#include <string>

#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>
#include <QVector>

namespace Res
{
    enum class Type {
        EditorTheme,
        Font,
        WindowTheme
    };

    struct DataPair {
        QString path;
        QString label;
    };

    inline const QString capitalizeName(QString path)
    {
        std::filesystem::path file_path = path.toStdString();
        auto stem = file_path.stem();
        auto name = QString::fromStdString(stem.string());
        auto name_capped = name.left(1).toUpper() + name.mid(1);
        return name_capped;
    }

    inline void collectResources(QDirIterator& iterator, Type resourceType, QVector<DataPair>& listOfPathPairs)
    {
        while (iterator.hasNext())
        {
            iterator.next();
            auto label = capitalizeName(iterator.filePath());
            (resourceType == Type::Font)
                ? listOfPathPairs << DataPair{ QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(iterator.filePath())).at(0), label }
                : listOfPathPairs << DataPair{ iterator.filePath(), label };
        }
    }

    inline const QVector<DataPair> iterateResources(QString path, QString ext, QString dataPath, Type resourceType)
    {
        QVector<DataPair> dataAndLabels;
        QDirIterator assets(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
        if (QDir(dataPath).exists())
        {
            QDirIterator user_assets(dataPath, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
            collectResources(user_assets, resourceType, dataAndLabels);
        }
        collectResources(assets, resourceType, dataAndLabels);
        std::sort(dataAndLabels.begin(), dataAndLabels.end(), [](auto& lhs, auto& rhs)
            {
                return lhs.label < rhs.label;
            });
        return dataAndLabels;
    }

    inline const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet)
    {
        QRegularExpressionMatchIterator matches = Uni::regex(Uni::Re::ThemeSheetLine).globalMatch(themeSheet);
        while (matches.hasNext())
        {
            QRegularExpressionMatch match = matches.next();
            if (match.hasMatch())
            {
                QString variable = match.captured(0).replace(Uni::regex(Uni::Re::ThemeSheetValue), nullptr);
                QString value = match.captured(0).replace(Uni::regex(Uni::Re::ThemeSheetVariable), nullptr);
                styleSheet.replace(QRegularExpression(variable), value);
            }
        }
        return styleSheet;
    }
}

// res.h, fernanda