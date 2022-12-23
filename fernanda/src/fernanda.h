// fernanda.h, fernanda

#pragma once

#include "colorbar.h"
#include "editor.h"
#include "indicator.h"
#include "pane.h"
#include "res.h"
#include "splitter.h"
#include "story.h"

#include <QAbstractButton>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsBlurEffect>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMoveEvent>
#include <QShowEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QSlider>
#include <QStatusBar>
#include <QStackedLayout>
#include <QTextOption>
#include <QTextTable>
#include <QWidgetAction>

class Fernanda : public QMainWindow
{
    using FsPath = std::filesystem::path;

    Q_OBJECT

public:
    Fernanda(bool dev, FsPath story, QWidget* parent = nullptr);

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
    QLabel* shadow = new QLabel(this);
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

    std::optional<Story> activeStory;
    bool isDev = false;
    bool isInitialized = false;
    bool hasStartUpBar = true;
    bool hasWinTheme = true;
    bool hasTheme = true;
    bool hasShadow = true;

    enum class Toggle {
        None = 0,
        Count,
        Pos,
        Theme,
        WinTheme
    };
    enum class WinStyle {
        BaseOnly,
        WithTheme
    };

    bool confirmStoryClose(bool isQuit = false);
    void openUd(FsPath path);
    const QStringList devPrintRenames(QVector<Io::ArcRename> renames);
    const QString ferName(bool dev = false);
    void addWidgets();
    QWidget* stackWidgets(QVector<QWidget*> widgets);
    void connections();
    void shortcuts();
    void makeMenuBar();
    void makeFileMenu();
    void makeSetMenu();
    void makeToggleMenu();
    void makeHelpMenu();
    void makeDevMenu();
    QActionGroup* makeViewToggles(QVector<Res::DataPair>& dataLabelPairs, void (Fernanda::* slot)());
    void loadConfigs(FsPath story);
    void loadWinConfigs();
    void loadViewConfig(QVector<QAction*> actions, Ud::ConfigGroup group, Ud::ConfigVal valueType, QVariant fallback);
    void loadMenuToggle(QAction* action, Ud::ConfigGroup group, Ud::ConfigVal valueType, QVariant fallback);
    void openStory(FsPath fileName, Story::Op opt = Story::Op::Normal);
    void actionCycle(QActionGroup* group);
    const QString windowStyle(WinStyle baseOnly = WinStyle::WithTheme);
    void toggleGlobals(bool& globalBool, Ud::ConfigGroup group, Ud::ConfigVal valueType, bool value, Toggle type = Toggle::None);
    void toggleWidget(QWidget* widget, Ud::ConfigGroup group, Ud::ConfigVal valueType, bool value);

private slots:
    void adjustTitle();
    void setWindowStyle();
    void setEditorStyle();
    void setEditorFont();
    void handleEditorZoom(TextEditor::Zoom direction);
    void aotToggled(bool checked);
    void setTabStop();
    void setWrapMode();
    void setBarAlignment();
    void fileSave();
    void helpMakeSampleProject();
    void helpMakeSampleRes();
    void helpShortcuts();
    void helpAbout();
    void devWrite(QString name, QString value);
    void handleEditorOpen(QString key = nullptr);
    void sendEditedText();
    bool replyHasProject();
    void domMove(QString pivotKey, QString fulcrumKey, Io::Move pos);
    void domAdd(QString newName, Path::Type type, QString parentKey);
    void domRename(QString newName, QString key);
    void domCut(QString key);
    void cycleCoreEditorThemes();
    void triggerOverlay(TextEditor::Overlay state);

signals:
    void sendColorBarToggle(bool checked);
    void sendLineHighlightToggle(bool checked);
    void sendKeyfilterToggle(bool checked);
    void sendLineNumberAreaToggle(bool checked);
    void sendScrollsToggle(bool checked);
    void sendExtraScrollsToggle(bool checked);
    void sendBlockCursorToggle(bool checked);
    void sendCursorBlinkToggle(bool checked);
    void sendItems(QVector<QStandardItem*> items);
    void sendEditsList(QVector<QString> editedFiles);
    void startAutoTempSave();
    void updatePositions(const int cursorBlockNumber, const int cursorPosInBlock);
    void updateCounts(const QString text, const int blockCount);
    void updateSelection(const QString selectedText, const int lineCount);
    void askEditorClose(bool isFinal = false);
};

// fernanda.h, fernanda