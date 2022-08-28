#pragma once

#include "pane.h"
#include "texteditor.h"
#include "version.h"

#include "algorithm"
#include "filesystem"
#include "functional"
#include "string"

#include <QtWidgets/QMainWindow>
#include "ui_fernanda.h"
#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QIODevice>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QModelIndex>
#include <QObject>
#include <QProgressBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextStream>
#include <QTimeLine>
#include <QTimer>
#include <QWidgetAction>

namespace fs = filesystem;

class Fernanda : public QMainWindow
{
    Q_OBJECT

public:
    Fernanda(QWidget *parent = nullptr);
    ~Fernanda() = default;

private:
    Ui::fernandaClass ui;

    const QString fernandaName = "fernanda";
    const QString userDataName = "." + fernandaName;
    const QString configName = fernandaName + ".ini";

    QStringList dirtyFiles; // move to texteditor?
    QString cleanText; // move to texteditor?
    
    QString defaultProjectsFolder;
    QString currentProject;
    fs::path userData;
    QString config;
    fs::path activeTemp;
    fs::path rollback;
    fs::path backup;

    bool wasInitialized = false;
    bool linePos = true;
    bool colPos = true;
    bool lineCount = true;
    bool wordCount = true;
    bool charCount = true;

    enum class PathType {
        Backup,
        Original,
        Rollback,
        Temp
    };
    enum class ResourceType {
        EditorTheme,
        Font,
        WindowTheme
    };
    enum class ColorScheme {
        None,
        Red,
        Green,
        StartUp
    };
    enum class ClearType {
        Full,
        Partial
    };

    QSplitter* splitter = new QSplitter(this);
    Pane* pane = new Pane(this);
    QLabel* overlay = new QLabel(this);
    TextEditor* textEditor = new TextEditor(this);
    QLabel* underlay = new QLabel(this);
    QProgressBar* colorBar = new QProgressBar(this);
    QTimer* barTimer = new QTimer(this);
    QLabel* pathDisplay = new QLabel(this);
    QLabel* positions = new QLabel(this);
    QLabel* counters = new QLabel(this);
    QPushButton* aot = new QPushButton(this);
    QTimer* autoSaveTimer = new QTimer(this);
    QActionGroup* windowThemes = new QActionGroup(this);
    QActionGroup* editorThemes = new QActionGroup(this);
    QActionGroup* editorFonts = new QActionGroup(this);
    QSlider* fontSlider = new QSlider(Qt::Horizontal);
    QActionGroup* tabStops = new QActionGroup(this);
    QActionGroup* wrapModes = new QActionGroup(this);

    void showEvent(QShowEvent* event);
    void nameObjects();
    void layoutObjects();
    void makeConnections();
    void checkTempsOnOpen();
    void closeEvent(QCloseEvent* event);
    void displayPath();
    void makeMenuBar();
    void makeFileMenu();
    void makeViewMenu();
    void makeHelpMenu();
    const QList<tuple<QString, QString>> resourceIterator(QString path, QString ext, ResourceType type);
    const QString resourceNameCap(QString path);
    QActionGroup* createMenuToggles(QList<tuple<QString, QString>>& itemAndLabel, void (Fernanda::* slot)());
    void createUserData(QString dataFolderName);
    void clearTempFiles();
    const QString readFile(QString path);
    const QString pathMaker(QString path, PathType type);
    void writeFile(QString text, QString path);
    void readWriteSampleFiles(QString path, fs::path subfolder);
    bool createUdThemesAndFonts();
    void tempSave(QString text, QString path);
    const QString tempOpen(QString path, QString tempPath);
    void swap(QString path);
    const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet);
    template<typename T> void saveConfig(QString group, QString valueName, T value);
    const QVariant loadConfig(QString group, QString valueName, bool preventRandomStrAsTrue = false);
    void loadResourceConfig(QList<QAction*> actions, QString group, QString valueName, QString fallback);
    void loadMiscConfigsOnOpen();
    bool loadProjectDataOnOpen();
    void resizeEvent(QResizeEvent* event);
    void moveEvent(QMoveEvent* event);
    void menuToggles(QWidget* widget, QString group, QString value, bool checked);
    void loadMenuToggles(QAction* action, QString group, QString value, bool fallback);
    void togglePosAndCounts(bool& globalBool, QString group, QString value, bool checked);
    void startColorBar(ColorScheme scheme);
    void setColorBarStyle(ColorScheme scheme);
    void clearAll(ClearType type);
    bool checkChildStatus(QString possibleParent, QString possibleChild);
    bool alert(ColorScheme scheme, QString message, QString optButton = "", void (Fernanda::* optButtonAction)() = nullptr, QString optReplaceMainButton = "", QString optReplaceTitle = "");

private slots:
    
    void chooseProjectDataOnOpen();
    void startUpColorBar();
    void textEditorTextChanged(); // Move to TextEditor?
    void fileMenuOpenProject();
    void fileMenuNewProject();
    void fileMenuNew();
    void fileMenuNewSubfolder();
    void fileMenuSave();
    void fileMenuSaveAll();
    void helpMenuSetProjectsDir();
    void helpMenuMakeSample();
    void helpMenuMakeSampleUdRc();
    void helpMenuAbout();
    void ifPreviousFileIsFile(QString path);
    void ifPathDoesNotEqualPrevPath(QString path);
    void setWindowStyle();
    void setEditorStyle();
    void setEditorFont();
    void setEditorFontSize(int value);
    void splitterResize();
    void aotToggled(bool checked);
    void startAutoSaveTimer();
    void autoSave();
    void toggleAOT(bool checked);
    void togglePane(bool checked);
    void toggleScrollPrevAndNext(bool checked);
    void toggleStatusbar(bool checked);
    void toggleLineHighlight(bool checked);
    void toggleKeyFilters(bool checked);
    void toggleLineNumberArea(bool checked);
    void updatePositions();
    void updateCounters();
    void toggleLinePos(bool checked);
    void toggleColPos(bool checked);
    void toggleLineCount(bool checked);
    void toggleWordCount(bool checked);
    void toggleCharCount(bool checked);
    void setTabStop();
    void setWrapMode();
    void colorBarTimerOver();
    void zoomFontSlider(bool zoomDirection);

signals:
    void sendLineHighlightToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
    void sendTabStop(int pixels);
    void sendWrapMode(QTextOption::WrapMode mode);
};

// fernanda.h, fernanda