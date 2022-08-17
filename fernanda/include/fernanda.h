#pragma once

#include "pane.h"
#include "texteditor.h"
#include "version.h"

#include "algorithm"
#include "filesystem"
#include "string"

#include <QtWidgets/QMainWindow>
#include "ui_fernanda.h"
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QIODevice>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QModelIndex>
#include <QObject>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSettings>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QString>
#include <QStringList>
#include <QTextCursor>
#include <QTextStream>
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
    fs::path sample;
    fs::path candide;
    fs::path candideSubfolder_1;
    fs::path candideSubfolder_2;
    fs::path candideSubfolder_3;
    fs::path candideSubfolder_4;

    bool linePos = true;
    bool colPos = true;
    bool lineCount = true;
    bool wordCount = true;
    bool charCount = true;

    enum class ChangeTo {
        BackupPath,
        OriginalPath,
        RollbackPath,
        TempPath
    };
    enum class Resource {
        EditorTheme,
        Font,
        WindowTheme
    };

    QSplitter* splitter = new QSplitter(this);
    Pane* pane = new Pane(this);
    TextEditor* textEditor = new TextEditor(this);
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

    void nameObjects();
    void layoutObjects();
    void makeConnections();
    void onOpen();
    void closeEvent(QCloseEvent* event);
    void hotkeys();
    void displayPath();
    void makeMenuBar();
    void makeFileMenu();
    void makeViewMenu();
    const QList<tuple<QString, QString>> resourceIterator(QString path, QString ext, Resource type);
    const QString resourceNameCap(QString path);
    QActionGroup* createSelectionMenu(QList<tuple<QString, QString>> itemAndLabel, void (Fernanda::* fx)());
    void createUserData(QString dataFolderName);
    void clearTempFiles();
    const QString readFile(QString path);
    const QString pathMaker(QString path, ChangeTo key);
    void writeFile(QString text, QString path);
    void createSample();
    void createSampleFiles();
    void readWriteSampleFiles(QString path, fs::path subfolder);
    void createUdThemesAndFonts(fs::path path);
    void tempSave(QString text, QString path);
    const QString tempOpen(QString path, QString tempPath);
    void swap(QString path);
    const QString createStyleSheetFromTheme(QString styleSheet, QString themeSheet);
    template<typename T> void saveConfig(QString group, QString value, T x);
    const QVariant loadConfig(QString group, QString value);
    void loadResourceConfig(QList<QAction*> actions, QString group, QString value, QString fallback);
    void loadConfigOnOpen();
    void resizeEvent(QResizeEvent* event);
    void moveEvent(QMoveEvent* event);
    void menuToggles(QWidget* widget, QString group, QString value, bool checked);
    void loadMenuToggles(QAction* action, QString group, QString value, bool fallback);
    void togglePosAndCounts(bool& globalBool, QString group, QString value, bool checked);

private slots:
    
    void textEditorTextChanged(); // move to texteditor?
    void fileMenuSave();
    void fileMenuSaveAll();
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

signals:
    void sendLineHighlightToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
    void sendTabStop(int pixels);
    void sendWrapMode(QTextOption::WrapMode mode);
};

// fernanda.h, fernanda