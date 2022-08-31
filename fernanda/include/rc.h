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
#include <QString>
#include <QStringList>

using namespace std;

enum class ResourceType {
    EditorTheme,
    Font,
    WindowTheme
};
const QList<tuple<QString, QString>> iterateResources(QString path, QString ext, filesystem::path dataPath, ResourceType type);
void collectResources(QDirIterator& iterator, ResourceType type, QList<tuple<QString, QString>>& listOfPathPairs);
const QString capitalizeName(QString path);
bool createSampleThemesAndFonts(filesystem::path dataFolder);
void makeSample(QString projectsDir);

// rc.h, fernanda