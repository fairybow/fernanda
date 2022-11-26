// fernanda.h, fernanda

#pragma once

#include "colorbar.h"
#include "editor.h"
#include "indicator.h"
#include "pane.h"
#include "project.h"
#include "res.h"
#include "splitter.h"

#include <QAbstractButton>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMoveEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSlider>
#include <QStatusBar>
#include <QStackedLayout>
#include <QTextOption>
#include <QWidgetAction>

class Fernanda : public QMainWindow
{
    Q_OBJECT

public:
    Fernanda(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QMenuBar* menuBar = new QMenuBar(this);
    QStatusBar* statusBar = new QStatusBar(this);
    Splitter* splitter = new Splitter(this);
    Pane* pane = new Pane(this);
    QLabel* overlay = new QLabel(this);
    TextEditor* textEditor = new TextEditor(this);
    QLabel* underlay = new QLabel(this);
    ColorBar* colorBar = new ColorBar(this);
    QActionGroup* windowThemes = new QActionGroup(this);
    QActionGroup* editorThemes = new QActionGroup(this);
    QActionGroup* editorFonts = new QActionGroup(this);
    QActionGroup* tabStops = new QActionGroup(this);
    QActionGroup* wrapModes = new QActionGroup(this);
    QActionGroup* barAlignments = new QActionGroup(this);
    QSlider* fontSlider = new QSlider(Qt::Horizontal);
    Indicator* indicator = new Indicator(this);
    QLabel* spacer = new QLabel(this);
    QPushButton* aot = new QPushButton(this);
    QTimer* autoTempSave = new QTimer(this);
    QLabel* test1 = new QLabel;
    QPushButton* test2 = new QPushButton;
    QPushButton* test3 = new QPushButton;

    const QString ferName = "fernanda";
    std::optional<Project> activeProject;
    bool isInitialized = false;
    bool hasStartUpBar = true;
    bool hasWinTheme = true;
    bool hasTheme = true;

    enum class Toggle {
        None = 0,
        Count,
        Pos,
        WinTheme,
        Theme
    };
    
    void addWidgets();
    QWidget* stackWidgets(QVector<QWidget*> widgets);
    void connections();
    void makeMenuBar();
    void makeFileMenu();
    void makeViewMenu(); // clean me
    void makeHelpMenu();
    QActionGroup* makeViewToggles(QVector<Res::DataPair>& dataLabelPairs, void (Fernanda::* slot)());
    void loadConfigs();
    void loadWinConfigs();
    void loadViewConfig(QVector<QAction*> actions, QString group, QString valueName, QVariant fallback);
    void loadMenuToggle(QAction* action, QString group, QString valueName, QVariant fallback);
    void openProject(QString fileName, Project::SP opt = Project::SP::None);

private slots:
    void setWindowStyle();
    void setEditorStyle();
    void setEditorFont();
    void handleEditorZoom(TextEditor::Zoom direction);
    void toggleWidget(QWidget* widget, QString group, QString valueName, bool value);
    void aotToggled(bool checked);
    void setTabStop();
    void setWrapMode();
    void setBarAlignment();
    void toggleGlobals(bool& globalBool, QString group, QString valueName, bool value, Toggle type = Toggle::None);
    void fileSave();
    //void helpProjects();
    void helpMakeSampleProject();
    void helpMakeSampleRes();
    void helpAbout();
    void handleEditorText(QString key);
    void sendEditedText();
    bool replyHasProject();
    void domMove(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void addElement(QString newName, Path::Type type, QString parentKey);
    void renameElement(QString newName, QString key);

signals:
    void sendColorBarToggle(bool checked);
    void sendLineHighlightToggle(bool checked);
    void sendKeyfilterToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
    void sendScrollsToggle(bool checked);
    void sendExtraScrollsToggle(bool checked);
    void sendInitExpansions(QVector<QString> initExpansions);
    void sendItems(QVector<QStandardItem*> items);
    void sendEditsList(QVector<QString> editedFiles);
    void startAutoTempSave();
    void updatePositions(const int cursorBlockNumber, const int cursorPosInBlock);
    void updateCounts(const QString text, const int blockCount);
    void updateSelection(const QString selectedText, const int lineCount);
};

// fernanda.h, fernanda