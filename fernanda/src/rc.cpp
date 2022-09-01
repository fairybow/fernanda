#include "rc.h"

const QList<std::tuple<QString, QString>> iterateResources(QString path, QString ext, std::filesystem::path dataPath, ResourceType type)
{
    QList<std::tuple<QString, QString>> dataAndLabels;
    QDirIterator assets(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
    if (QDir(dataPath).exists())
    {
        QDirIterator user_assets(QString::fromStdString(dataPath.string()), QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
        collectResources(user_assets, type, dataAndLabels);
    }
    collectResources(assets, type, dataAndLabels);
    std::sort(dataAndLabels.begin(), dataAndLabels.end(), [](auto& lhs, auto& rhs)
        {
            return std::get<1>(lhs) < std::get<1>(rhs);
        });
    return dataAndLabels;
}

void collectResources(QDirIterator& iterator, ResourceType type, QList<std::tuple<QString, QString>>& listOfPathPairs)
{
    while (iterator.hasNext())
    {
        iterator.next();

        auto label = capitalizeName(iterator.filePath());

        if (type == ResourceType::Font)
            listOfPathPairs << std::tuple<QString, QString>(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(iterator.filePath())).at(0), label);
        else
            listOfPathPairs << std::tuple<QString, QString>(iterator.filePath(), label);
    }
}

const QString capitalizeName(QString path)
{
    std::filesystem::path file_path = path.toStdString();
    auto stem = file_path.stem();
    auto name = QString::fromStdString(stem.string());
    auto name_capped = name.left(1).toUpper() + name.mid(1);
    return name_capped;
}

bool createSampleThemesAndFonts(std::filesystem::path dataFolder)
{
    auto font_fs = dataFolder / "Merriweather.ttf";
    auto editor_theme_fs = dataFolder / "sample.fernanda_theme";
    auto win_theme_fs = dataFolder / "sample.fernanda_wintheme";
    auto font = QString::fromStdString(font_fs.string());
    auto editor_theme = QString::fromStdString(editor_theme_fs.string());
    auto win_theme = QString::fromStdString(win_theme_fs.string());
    if (QFile(font).exists() || QFile(editor_theme).exists() || QFile(win_theme).exists())
        return false;
    QFile::copy(":/sample/Merriweather.ttf", font);
    QFile::copy(":/sample/sample.fernanda_theme", editor_theme);
    QFile::copy(":/sample/sample.fernanda_wintheme", win_theme);
    QFile(font).setPermissions(QFile::WriteUser);
    QFile(editor_theme).setPermissions(QFile::WriteUser);
    QFile(win_theme).setPermissions(QFile::WriteUser);
    return true;
}

void makeSample(QString projectsDir)
{
    QDirIterator files_1(":/sample/Candide/Chapters 1-10/", QStringList() << "*.txt", QDir::Files);
    QDirIterator files_2(":/sample/Candide/Chapters 11-20/", QStringList() << "*.txt", QDir::Files);
    QDirIterator files_3(":/sample/Candide/Chapters 21-30/", QStringList() << "*.txt", QDir::Files);
    QDirIterator files_4(":/sample/Candide/Misc/", QStringList() << "*.txt", QDir::Files);
    std::filesystem::path projects_folder = projectsDir.toStdString();
    std::filesystem::path candide = projects_folder / "Candide";
    auto subfolder_1 = candide / std::string("Chapters 1-10");
    auto subfolder_2 = candide / std::string("Chapters 11-20");
    auto subfolder_3 = candide / std::string("Chapters 21-30");
    auto subfolder_4 = candide / std::string("Misc");
    QList<std::filesystem::path> sample_folders = { candide, subfolder_1, subfolder_2, subfolder_3, subfolder_4 };
    for (auto& folder : sample_folders)
        std::filesystem::create_directory(folder);
    QList<std::tuple<QDirIterator&, std::filesystem::path>> iteratorAndPathPairs = {
        std::tuple<QDirIterator&, std::filesystem::path>(files_1, subfolder_1),
        std::tuple<QDirIterator&, std::filesystem::path>(files_2, subfolder_2),
        std::tuple<QDirIterator&, std::filesystem::path>(files_3, subfolder_3),
        std::tuple<QDirIterator&, std::filesystem::path>(files_4, subfolder_4),
    };
    for (auto& pair : iteratorAndPathPairs)
    {
        auto& iterator = std::get<0>(pair);
        auto& folder_path = std::get<1>(pair);
        while (iterator.hasNext())
        {
            iterator.next();
            auto file = iterator.filePath();
            auto name = iterator.fileName();
            auto path = folder_path / name.toStdString();
            auto copy_path = QString::fromStdString(path.string());
            QFile::copy(file, copy_path);
            QFile(copy_path).setPermissions(QFile::WriteUser);
        }
    }
}

const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet)
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

// rc.cpp, fernanda