// res.h, fernanda

#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <tuple>

#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>
#include <QVector>

namespace RC
{
    enum class ResType {
        EditorTheme,
        Font,
        WindowTheme
    };

    inline const QString capitalizeName(QString path)
    {
        std::filesystem::path file_path = path.toStdString();
        auto stem = file_path.stem();
        auto name = QString::fromStdString(stem.string());
        auto name_capped = name.left(1).toUpper() + name.mid(1);
        return name_capped;
    }

    inline void collectResources(QDirIterator& iterator, ResType resourceType, QVector<std::tuple<QString, QString>>& listOfPathPairs)
    {
        while (iterator.hasNext())
        {
            iterator.next();
            auto label = RC::capitalizeName(iterator.filePath());
            if (resourceType == ResType::Font)
                listOfPathPairs << std::tuple<QString, QString>(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(iterator.filePath())).at(0), label);
            else
                listOfPathPairs << std::tuple<QString, QString>(iterator.filePath(), label);
        }
    }

    inline const QVector<std::tuple<QString, QString>> iterateResources(QString path, QString ext, QString dataPath, ResType resourceType)
    {
        QVector<std::tuple<QString, QString>> dataAndLabels;
        QDirIterator assets(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
        if (QDir(dataPath).exists())
        {
            QDirIterator user_assets(dataPath, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
            RC::collectResources(user_assets, resourceType, dataAndLabels);
        }
        RC::collectResources(assets, resourceType, dataAndLabels);
        std::sort(dataAndLabels.begin(), dataAndLabels.end(), [](auto& lhs, auto& rhs)
            {
                return std::get<1>(lhs) < std::get<1>(rhs);
            });
        return dataAndLabels;
    }

    inline const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet)
    {
        QRegularExpressionMatchIterator matches = QRegularExpression("(@.*\\n?)").globalMatch(themeSheet);
        while (matches.hasNext())
        {
            QRegularExpressionMatch match = matches.next();
            if (match.hasMatch())
            {
                QString variable = match.captured(0).replace(QRegularExpression("(\\s=.*;)"), "");
                QString value = match.captured(0).replace(QRegularExpression("(@.*=\\s)"), "");
                styleSheet.replace(QRegularExpression(variable), value);
            }
        }
        return styleSheet;
    }
}

// res.h, fernanda