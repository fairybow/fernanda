#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <tuple>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFontDatabase>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>

enum class ResourceType {
    EditorTheme,
    Font,
    WindowTheme
};
const QList<std::tuple<QString, QString>> iterateResources(QString path, QString ext, std::filesystem::path dataPath, ResourceType type);
void collectResources(QDirIterator& iterator, ResourceType type, QList<std::tuple<QString, QString>>& listOfPathPairs);
const QString capitalizeName(QString path);
bool createSampleThemesAndFonts(std::filesystem::path dataFolder);
void makeSample(QString projectsDir);
const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet);

// rc.h, fernanda