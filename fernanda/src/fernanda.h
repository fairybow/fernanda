// fernanda.h, fernanda

#pragma once

#include "editor.h"
#include "pane.h"
#include "project.h"
#include "res.h"
#include "ui_fernanda.h"

#include <QAbstractButton>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFileDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QMetaObject>
#include <QMoveEvent>
#include <QProgressBar>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSlider>
#include <QSplitter>
#include <QStackedLayout>
#include <QTextOption>
#include <QTimeLine>
#include <QTimer>
#include <QWidgetAction>

class Fernanda : public QMainWindow
{
    Q_OBJECT

public:
    Fernanda(QWidget* parent = nullptr);
    ~Fernanda() = default;

private:
    Ui::fernandaClass ui;
    const QString ferName = "fernanda";
    //const QString ferName = "fernanda-dev";
    std::optional<Project> currentProject;
    QSplitter* splitter = new QSplitter(this);
    Pane* pane = new Pane(this);
    QLabel* overlay = new QLabel(this);
    TextEditor* textEditor = new TextEditor(this);
    QLabel* underlay = new QLabel(this);
    QProgressBar* colorBar = new QProgressBar(this);
    QTimer* barTimer = new QTimer(this);
    QActionGroup* windowThemes = new QActionGroup(this);
    QActionGroup* editorThemes = new QActionGroup(this);
    QActionGroup* editorFonts = new QActionGroup(this);
    QActionGroup* tabStops = new QActionGroup(this);
    QActionGroup* wrapModes = new QActionGroup(this);
    QSlider* fontSlider = new QSlider(Qt::Horizontal);
    QLabel* positions = new QLabel(this);
    QLabel* counters = new QLabel(this);
    QLabel* spacer = new QLabel(this);
    QPushButton* aot = new QPushButton(this);
    QLabel* test1 = new QLabel(this);
    QPushButton* test2 = new QPushButton(this);
    QPushButton* test3 = new QPushButton(this);

    bool isInitialized = false;
    bool hasWinTheme = true;
    bool hasTheme = true;
    bool linePos = true;
    bool colPos = true;
    bool lineCount = true;
    bool wordCount = true;
    bool charCount = true;

    enum class Color {
        None = 0,
        Green,
        Red,
        StartUp
    };
    enum class Toggle {
        Count,
        Pos,
        WinTheme,
        Theme
    };
    
    void tests();
    void showEvent(QShowEvent* event);
    void resizeEvent(QResizeEvent* event);
    void moveEvent(QMoveEvent* event);
    void closeEvent(QCloseEvent* event);
    void handleQtUi();
    void addWidgets();
    void connections();
    void makeMenuBar();
    void makeFileMenu();
    void makeViewMenu(); // clean me
    void makeHelpMenu();
    QActionGroup* makeViewToggles(QVector<std::tuple<QString, QString>>& itemAndLabelPairs, void (Fernanda::* slot)());
    void colorBarOn(Color style = Color::None);
    void colorBarStyle(Color style);
    void loadOpeningConfigs();
    void loadWindowConfigs();
    void loadViewConfig(QVector<QAction*> actions, QString group, QString valueName, QVariant fallback);
    void loadViewConfigToggle(QAction* action, QString group, QString valueName, QVariant fallback);
    void openProject(QString fileName);

private slots:
    void setWindowStyle();
    void setEditorStyle();
    void setEditorFont();
    void toggleWidget(QWidget* widget, QString group, QString valueName, bool value);
    void aotToggled(bool checked);
    void setTabStop();
    void setWrapMode();
    void toggleGlobals(bool& globalBool, QString group, QString valueName, bool value, Toggle type);
    void updatePositions();
    void updateCounters();
    void fileSave();
    void fileSaveAll();
    //void helpProjects();
    void helpMakeSampleRes();
    void helpMakeSampleProject();
    void helpAbout();
    void handleEditorText(QString relFilePath);
    void sendEditedText();
    bool replyHasProject();

signals:
    void sendLineHighlightToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
    void sendItems(QVector<QStandardItem*> itemsVector);
    void sendEditsList(QVector<QString> editedFiles);
    void saveCursors(QString relFilePath);
    void applyCursors(QString relFilePath);
    void saveUndoStacks(QString relFilePath);
    void applyUndoStacks(QString relFilePath);
};

// fernanda.h, fernanda