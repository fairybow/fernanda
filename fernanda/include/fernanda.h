#pragma once

#include "io.h"
#include "pane.h"
#include "rc.h"
#include "texteditor.h"
//#include "version.h"

#include <functional>

#include <QtWidgets/QMainWindow>
#include "ui_fernanda.h"
#include <QAbstractButton>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QObject>
#include <QProgressBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QTextCursor>
#include <QTimeLine>
#include <QTimer>
#include <QWidgetAction>

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
    std::filesystem::path userData;
    QString config;
    std::filesystem::path activeTemp;
    std::filesystem::path rollback;
    std::filesystem::path backup;

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
    QActionGroup* createMenuToggles(QList<std::tuple<QString, QString>>& itemAndLabel, void (Fernanda::* slot)());
    void createUserData(QString dataFolderName);
    void clearTempFiles();
    const QString pathMaker(QString path, PathType type);
    void swap(QString path);
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
    bool alert(ColorScheme scheme, QString message, QString optButton = "", void (Fernanda::* optButtonAction)() = nullptr, QString optReplaceMainButton = "", QString optReplaceTitle = "");

private slots:
    
    void chooseProjectDataOnOpen();
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
    void aotToggled(bool checked);
    void updatePositions();
    void updateCounters(); 
    void setTabStop();
    void setWrapMode();

signals:
    void sendLineHighlightToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
};

// fernanda.h, fernanda