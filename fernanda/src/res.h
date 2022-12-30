// res.h, Fernanda

#pragma once

#include "path.h"
#include "uni.h"

#include <algorithm>

#include <QDirIterator>
#include <QFontDatabase>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QStringList>
#include <QVector>

namespace Res
{
    namespace Fs = std::filesystem;

    enum class Type {
        EditorTheme,
        Font,
        WindowTheme
    };

    struct DataPair {
        Fs::path path;
        QString label;
    };

    inline void collectResources(QDirIterator& iterator, Type resourceType, QVector<DataPair>& listOfPathPairs)
    {
        while (iterator.hasNext())
        {
            iterator.next();
            auto q_path = iterator.filePath();
            Fs::path path;
            (resourceType == Type::Font)
                ? path = Path::toFs(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(q_path)).at(0))
                : path = Path::toFs(q_path);
            auto label = Path::getName<QString>(q_path);
            listOfPathPairs << DataPair{ path, label };
        }
    }

    inline const QVector<DataPair> iterateResources(Fs::path path, QString ext, Fs::path dataPath, Type resourceType, QVector<DataPair> existingList = QVector<DataPair>())
    {
        QVector<DataPair> dataAndLabels;
        if (!existingList.isEmpty())
            dataAndLabels << existingList;
        QDirIterator assets(Path::toQString(path), QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
        if (QDir(dataPath).exists())
        {
            QDirIterator user_assets(Path::toQString(dataPath), QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
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

// res.h, Fernanda
