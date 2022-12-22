// fernanda.cpp, fernanda

#include "fernanda.h"

Fernanda::Fernanda(bool dev, FsPath story, QWidget* parent)
    : QMainWindow(parent)
{
    auto name = ferName(dev);
    setWindowTitle(name);
    Ud::setName(name);
    //Ud::windowsReg();
    addWidgets();
    connections();
    Ud::userData();
    makeMenuBar();
    loadConfigs(story);
}

void Fernanda::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (isInitialized || event->spontaneous()) return;
    if (hasStartUpBar)
        colorBar->delayedStartUp();
    isInitialized = true;
}

void Fernanda::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::Position, geometry());
}

void Fernanda::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::Position, geometry());
}

void Fernanda::closeEvent(QCloseEvent* event)
{
    auto state = windowState();
    Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::State, state.toInt());
    setWindowState(Qt::WindowState::WindowActive);
    auto quit = confirmStoryClose(true);
    if (!quit)
    {
        setWindowState(state);
        event->ignore();
        colorBar->green();
        return;
    }
    Ud::clear(Ud::userData(Ud::Op::GetTemp), true);
    event->accept();
}

bool Fernanda::confirmStoryClose(bool isQuit)
{
    if (!activeStory.has_value() || !activeStory.value().hasChanges()) return true;
    QMessageBox alert;
    alert.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    alert.setWindowTitle("Hey!");
    alert.setText(Uni::change(isQuit));
    alert.addButton(QMessageBox::Yes);
    auto no = alert.addButton(QMessageBox::No);
    auto save_and = alert.addButton(tr(Uni::saveAnd(isQuit)), QMessageBox::ActionRole);
    alert.setDefaultButton(no);
    alert.exec();
    if (alert.clickedButton() == no)
        return false;
    else if (alert.clickedButton() == save_and)
        fileSave();
    return true;
}

void Fernanda::openUd(FsPath path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(Path::toQString(path)));
}

const QStringList Fernanda::devPrintRenames(QVector<Io::ArcRename> renames)
{
    QStringList result;
    auto i = 0;
    for (auto& rename : renames)
    {
        ++i;
        QString entry = QString::number(i) + "\nKey: " + rename.key + "\nRel Path: " + Path::toQString(rename.relPath);
        (rename.origRelPath.has_value())
            ? entry = entry + "\nOrig Path: " + Path::toQString(rename.origRelPath.value())
            : entry = entry + "\nNew: " + QString((rename.typeIfNewOrCut.value() == Path::Type::Dir) ? "dir" : "file");
        result << entry;
    }
    return result;
}

const QString Fernanda::ferName(bool dev)
{
    QString result;
    if (dev)
        isDev = dev;
    (isDev)
        ? result = "fernanda (dev)"
        : result = "fernanda";
    return result;
}

void Fernanda::addWidgets()
{
    auto main_container = stackWidgets({ colorBar, splitter });
    auto editor_container = stackWidgets({ overlay, textEditor, underlay });
    splitter->addWidgets({ pane, editor_container });
    statusBar->setSizeGripEnabled(true);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    underlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMenuBar(menuBar);
    setStatusBar(statusBar);
    setCentralWidget(main_container);
    aot->setCheckable(true);
    aot->setText(Uni::ico(Uni::Ico::Pushpin));
    auto aotEffect = new QGraphicsOpacityEffect(this);
    aotEffect->setOpacity(0.8);
    aot->setGraphicsEffect(aotEffect);
    statusBar->addPermanentWidget(indicator, 0);
    statusBar->addPermanentWidget(spacer, 1);
    statusBar->addPermanentWidget(aot, 0);
    statusBar->setMaximumHeight(22);
    menuBar->setObjectName("menuBar");
    statusBar->setObjectName("statusBar");
    overlay->setObjectName("overlay");
    underlay->setObjectName("underlay");
    fontSlider->setObjectName("fontSlider");
    spacer->setObjectName("spacer");
    aot->setObjectName("aot");
}

QWidget* Fernanda::stackWidgets(QVector<QWidget*> widgets)
{
    QWidget* container = new QWidget(this);
    QStackedLayout* stack_layout = new QStackedLayout(container);
    stack_layout->setStackingMode(QStackedLayout::StackAll);
    for (auto& widget : widgets)
        stack_layout->addWidget(widget);
    return container;
}

void Fernanda::connections()
{
    shortcuts();
    connect(this, &Fernanda::updatePositions, indicator, &Indicator::updatePositions);
    connect(this, &Fernanda::updateCounts, indicator, &Indicator::updateCounts);
    connect(this, &Fernanda::updateSelection, indicator, &Indicator::updateSelection);
    connect(this, &Fernanda::sendColorBarToggle, colorBar, &ColorBar::toggleSelf);
    connect(this, &Fernanda::sendLineHighlightToggle, textEditor, &TextEditor::toggleLineHighlight);
    connect(this, &Fernanda::sendKeyfilterToggle, textEditor, &TextEditor::toggleKeyfilter);
    connect(this, &Fernanda::sendLineNumberAreaToggle, textEditor, &TextEditor::toggleLineNumberArea);
    connect(this, &Fernanda::sendScrollsToggle, textEditor, &TextEditor::toggleScrolls);
    connect(this, &Fernanda::sendExtraScrollsToggle, textEditor, &TextEditor::toggleExtraScrolls);
    connect(this, &Fernanda::sendBlockCursorToggle, textEditor, &TextEditor::toggleBlockCursor);
    connect(this, &Fernanda::sendCursorBlinkToggle, textEditor, &TextEditor::toggleCursorBlink);
    connect(this, &Fernanda::askEditorClose, textEditor, &TextEditor::close);
    connect(this, &Fernanda::sendItems, pane, &Pane::receiveItems);
    connect(this, &Fernanda::sendEditsList, pane, &Pane::receiveEditsList);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
    connect(pane, &Pane::askDomMove, this, &Fernanda::domMove);
    connect(pane, &Pane::askAddElement, this, &Fernanda::domAdd);
    connect(pane, &Pane::askRenameElement, this, &Fernanda::domRename);
    connect(pane, &Pane::askCutElement, this, &Fernanda::domCut);
    connect(pane, &Pane::askHasProject, this, &Fernanda::replyHasProject);
    connect(pane, &Pane::askSendToEditor, this, &Fernanda::handleEditorOpen);
    connect(textEditor, &TextEditor::askFontSliderZoom, this, &Fernanda::handleEditorZoom);
    connect(textEditor, &TextEditor::askHasProject, this, &Fernanda::replyHasProject);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::sendEditedText);
    connect(textEditor, &TextEditor::askOverlay, this, &Fernanda::triggerOverlay);
    connect(this, &Fernanda::startAutoTempSave, this, [&]() { autoTempSave->start(30000); });
    connect(autoTempSave, &QTimer::timeout, this, [&]() { activeStory.value().autoTempSave(textEditor->toPlainText()); });
    connect(pane, &Pane::askSetExpansion, this, [&](QString key, bool isExpanded) { activeStory.value().setItemExpansion(key, isExpanded); });
    connect(textEditor, &TextEditor::askNavNext, pane, [&]() { pane->nav(Pane::Nav::Next); });
    connect(textEditor, &TextEditor::askNavPrevious, pane, [&]() { pane->nav(Pane::Nav::Previous); });
    connect(this, &Fernanda::addStoryToTitle, this, [&](FsPath path)
        {
            setWindowTitle(Path::getName(path) + " - " + ferName());
        });
    connect(textEditor, &TextEditor::cursorPositionChanged, this, [&]()
        {
            updatePositions(textEditor->textCursor().blockNumber(), textEditor->textCursor().positionInBlock());
        });
    connect(textEditor, &TextEditor::textChanged, this, [&]()
        {
            updateCounts(textEditor->toPlainText(), textEditor->blockCount());
        });
    connect(textEditor, &TextEditor::selectionChanged, this, [&]()
        {
            (textEditor->textCursor().hasSelection())
                ? updateSelection(textEditor->textCursor().selectedText(), textEditor->selectedLineCount())
                : textEditor->textChanged();
        });
}

void Fernanda::shortcuts()
{
    connect(new QShortcut(Qt::Key_F11, this), &QShortcut::activated, this, &Fernanda::cycleCoreEditorThemes);
    connect(new QShortcut(Qt::ALT | Qt::Key_F10, this), &QShortcut::activated, this, [&]() { actionCycle(editorFonts); });
    connect(new QShortcut(Qt::ALT | Qt::Key_F11, this), &QShortcut::activated, this, [&]() { actionCycle(editorThemes); });
    connect(new QShortcut(Qt::ALT | Qt::Key_F12, this), &QShortcut::activated, this, [&]() { actionCycle(windowThemes); });
    connect(new QShortcut(Qt::ALT | Qt::Key_Insert, this), &QShortcut::activated, this, [&]()
        {
            textEditor->scrollNavClicked(TextEditor::Scroll::Previous);
        });
    connect(new QShortcut(Qt::ALT | Qt::Key_Delete, this), &QShortcut::activated, this, [&]()
        {
            textEditor->scrollNavClicked(TextEditor::Scroll::Next);
        });
    connect(new QShortcut(Qt::ALT | Qt::Key_Minus, this), &QShortcut::activated, this, [&]()
        {
            handleEditorZoom(TextEditor::Zoom::Out);
        });
    connect(new QShortcut(Qt::ALT | Qt::Key_Equal, this), &QShortcut::activated, this, [&]()
        {
            handleEditorZoom(TextEditor::Zoom::In);
        });
}

void Fernanda::makeMenuBar()
{
    makeFileMenu();
    makeSetMenu();
    makeToggleMenu();
    makeHelpMenu();
    if (!isDev) return;
    makeDevMenu();
}

void Fernanda::makeFileMenu()
{
    auto* new_story = new QAction(tr("&New project..."), this);
    auto* open_story = new QAction(tr("&Open an existing project..."), this);
    auto* save = new QAction(tr("&Save"), this);
    auto* quit = new QAction(tr("&Quit"), this);
    save->setShortcut(Qt::CTRL | Qt::Key_S);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(new_story, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new story..."), Path::toQString(Ud::userData(Ud::Op::GetDocs)), tr("Fernanda story file (*.story)"));
            openStory(Path::toFs(file_name));
        });
    connect(open_story, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getOpenFileName(this, tr("Open an existing story..."), Path::toQString(Ud::userData(Ud::Op::GetDocs)), tr("Fernanda story file (*.story)"));
            openStory(Path::toFs(file_name));
        });
    connect(save, &QAction::triggered, this, &Fernanda::fileSave);
    connect(quit, &QAction::triggered, this, &QCoreApplication::quit, Qt::QueuedConnection);
    auto* file = menuBar->addMenu(tr("&File"));
    file->addAction(new_story);
    file->addAction(open_story);
    file->addSeparator();
    file->addAction(save);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeSetMenu()
{
    auto user_data = Ud::userData(Ud::Op::GetUserData);
    QVector<Res::DataPair> bar_alignments = {
        Res::DataPair{ "Top", "Top" },
        Res::DataPair{ "Bottom", "Bottom" }
    };
    auto win_theme_list = Res::iterateResources(":/themes/window/", "*.fernanda_wintheme", user_data, Res::Type::WindowTheme);
    QVector<Res::DataPair> font_list;
    auto ttfs = Res::iterateResources(":/fonts/", "*.ttf", user_data, Res::Type::Font);
    font_list << Res::iterateResources(":/fonts/", "*.otf", user_data, Res::Type::Font, ttfs);
    auto editor_theme_list = Res::iterateResources(":/themes/editor/", "*.fernanda_theme", user_data, Res::Type::EditorTheme);
    QVector<Res::DataPair> tab_list = {
        Res::DataPair{ "20", "20 px" },
        Res::DataPair{ "40", "40 px" },
        Res::DataPair{ "60", "60 px" },
        Res::DataPair{ "80", "80 px" }
    };
    QVector<Res::DataPair> wrap_list = {
        Res::DataPair{ "NoWrap", "No wrap" },
        Res::DataPair{ "WordWrap", "Wrap at word boundaries" },
        Res::DataPair{ "WrapAnywhere", "Wrap anywhere" },
        Res::DataPair{ "WrapAt", "Wrap at word boundaries or anywhere" }
    };
    barAlignments = makeViewToggles(bar_alignments, &Fernanda::setBarAlignment);
    auto* toggle_col_pos = new QAction(tr("&Column position"), this);
    auto* toggle_line_pos = new QAction(tr("&Line position"), this);
    auto* toggle_char_count = new QAction(tr("&Character count"), this);
    auto* toggle_line_count = new QAction(tr("&Line count"), this);
    auto* toggle_word_count = new QAction(tr("&Word count"), this);
    windowThemes = makeViewToggles(win_theme_list, &Fernanda::setWindowStyle);
    editorFonts = makeViewToggles(font_list, &Fernanda::setEditorFont);
    auto* font_size_label = new QAction(tr("&Editor font size:"), this);
    auto* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(40);
    editorThemes = makeViewToggles(editor_theme_list, &Fernanda::setEditorStyle);
    tabStops = makeViewToggles(tab_list, &Fernanda::setTabStop);
    wrapModes = makeViewToggles(wrap_list, &Fernanda::setWrapMode);
    connect(toggle_col_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.colPos, Ud::ConfigGroup::Window, Ud::ConfigVal::PosCol, checked, Toggle::Pos);
        });
    connect(toggle_line_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.linePos, Ud::ConfigGroup::Window, Ud::ConfigVal::PosLine, checked, Toggle::Pos);
        });
    connect(toggle_char_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.charCount, Ud::ConfigGroup::Window, Ud::ConfigVal::CountChar, checked, Toggle::Count);
        });
    connect(toggle_line_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.lineCount, Ud::ConfigGroup::Window, Ud::ConfigVal::CountLine, checked, Toggle::Count);
        });
    connect(toggle_word_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.wordCount, Ud::ConfigGroup::Window, Ud::ConfigVal::CountWord, checked, Toggle::Count);
        });
    connect(fontSlider, &QSlider::valueChanged, this, [&](int value)
        {
            setEditorFont();
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::FontSize, value);
        });
    for (auto& action : {
        toggle_col_pos,
        toggle_line_pos,
        toggle_char_count,
        toggle_line_count,
        toggle_word_count
        })
        action->setCheckable(true);
    font_size_label->setEnabled(false);
    loadViewConfig(barAlignments->actions(), Ud::ConfigGroup::Window, Ud::ConfigVal::BarAlign, "Top");
    loadMenuToggle(toggle_col_pos, Ud::ConfigGroup::Window, Ud::ConfigVal::PosCol, true);
    loadMenuToggle(toggle_line_pos, Ud::ConfigGroup::Window, Ud::ConfigVal::PosLine, true);
    loadMenuToggle(toggle_char_count, Ud::ConfigGroup::Window, Ud::ConfigVal::CountChar, false);
    loadMenuToggle(toggle_line_count, Ud::ConfigGroup::Window, Ud::ConfigVal::CountLine, true);
    loadMenuToggle(toggle_word_count, Ud::ConfigGroup::Window, Ud::ConfigVal::CountWord, true);
    loadViewConfig(windowThemes->actions(), Ud::ConfigGroup::Window, Ud::ConfigVal::WinTheme, ":/themes/window/Light.fernanda_wintheme");
    loadViewConfig(editorFonts->actions(), Ud::ConfigGroup::Editor, Ud::ConfigVal::Font, "Cascadia Mono");
    loadViewConfig(editorThemes->actions(), Ud::ConfigGroup::Editor, Ud::ConfigVal::EditorTheme, ":/themes/editor/Amber.fernanda_theme");
    loadViewConfig(tabStops->actions(), Ud::ConfigGroup::Editor, Ud::ConfigVal::TabStop, "40");
    loadViewConfig(wrapModes->actions(), Ud::ConfigGroup::Editor, Ud::ConfigVal::Wrap, "WrapAt");
    auto* set = menuBar->addMenu(tr("&Set"));
    auto* bar_alignment = set->addMenu(tr("&Color bar alignment"));
    bar_alignment->addActions(barAlignments->actions());
    auto* indicator_items = set->addMenu(tr("&Indicator"));
    indicator_items->addAction(toggle_col_pos);
    indicator_items->addAction(toggle_line_pos);
    indicator_items->addSeparator();
    indicator_items->addAction(toggle_char_count);
    indicator_items->addAction(toggle_line_count);
    indicator_items->addAction(toggle_word_count);
    auto* window_themes = set->addMenu(tr("&Window theme"));
    window_themes->addActions(windowThemes->actions());
    set->addSeparator();
    auto* fonts = set->addMenu(tr("&Editor font"));
    fonts->addActions(editorFonts->actions());
    set->addAction(font_size_label);
    set->addAction(font_size);
    auto* editor_themes = set->addMenu(tr("&Editor theme"));
    editor_themes->addActions(editorThemes->actions());
    auto* tab_stops = set->addMenu(tr("&Tab stop distance"));
    tab_stops->addActions(tabStops->actions());
    auto* wrap_modes = set->addMenu(tr("&Wrap mode"));
    wrap_modes->addActions(wrapModes->actions());
}

void Fernanda::makeToggleMenu()
{
    auto* toggle_aot = new QAction(tr("&Always-on-top button"), this);
    auto* toggle_bar = new QAction(tr("&Color bar"), this);
    auto* toggle_indicator = new QAction(tr("&Indicator"), this);
    auto* toggle_pane = new QAction(tr("&Pane"), this);
    auto* toggle_statusbar = new QAction(tr("&Status bar"), this);
    auto* toggle_win_theme = new QAction(tr("&Window theme"), this);
    auto* toggle_cursor_blink = new QAction(tr("&Blink"), this);
    auto* toggle_block_cursor = new QAction(tr("&Block"), this);
    auto* toggle_line_highlight = new QAction(tr("&Current line highlight"), this);
    auto* toggle_shadow = new QAction(tr("&Editor shadow"), this);
    auto* toggle_theme = new QAction(tr("&Editor theme"), this);
    auto* toggle_keyfilter = new QAction(tr("&Key filters"), this);
    auto* toggle_line_numbers = new QAction(tr("&Line number area"), this);
    auto* toggle_scrolls = new QAction(tr("&Scrolls previous and next"), this);
    auto* load_recent = new QAction(tr("&Load most recent project on open"), this);
    connect(toggle_aot, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(aot, Ud::ConfigGroup::Window, Ud::ConfigVal::T_AotBtn, checked);
        });
    connect(toggle_bar, &QAction::toggled, this, [&](bool checked)
        {
            sendColorBarToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::T_ColorBar, checked);
        });
    connect(toggle_indicator, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(indicator, Ud::ConfigGroup::Window, Ud::ConfigVal::T_Indicator, checked);
        });
    connect(toggle_pane, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(pane, Ud::ConfigGroup::Window, Ud::ConfigVal::T_Pane, checked);
        });
    connect(toggle_statusbar, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(statusBar, Ud::ConfigGroup::Window, Ud::ConfigVal::T_StatusBar, checked);
        });
    connect(toggle_win_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasWinTheme, Ud::ConfigGroup::Window, Ud::ConfigVal::T_WinTheme, checked, Toggle::WinTheme);
        });
    connect(toggle_cursor_blink, &QAction::toggled, this, [&](bool checked)
        {
            sendCursorBlinkToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_CursorBlink, checked);
        });
    connect(toggle_block_cursor, &QAction::toggled, this, [&](bool checked)
        {
            sendBlockCursorToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Cursor, checked);
        });
    connect(toggle_line_highlight, &QAction::toggled, this, [&](bool checked)
        {
            sendLineHighlightToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_LineHighlight, checked);
        });
    connect(toggle_shadow, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasShadow, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Shadow, checked, Toggle::Theme);
        });
    connect(toggle_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasTheme, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_EditorTheme, checked, Toggle::Theme);
        });
    connect(toggle_keyfilter, &QAction::toggled, this, [&](bool checked)
        {
            sendKeyfilterToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Keyfilter, checked);
        });
    connect(toggle_line_numbers, &QAction::toggled, this, [&](bool checked)
        {
            sendLineNumberAreaToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Lna, checked);
        });
    connect(toggle_scrolls, &QAction::toggled, this, [&](bool checked)
        {
            sendExtraScrollsToggle(checked);
            Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Nav, checked);
        });
    connect(load_recent, &QAction::toggled, this, [&](bool checked)
        {
            Ud::saveConfig(Ud::ConfigGroup::Data, Ud::ConfigVal::T_Lmr, checked);
        });
    for (auto& action : {
        toggle_aot,
        toggle_bar,
        toggle_indicator,
        toggle_pane,
        toggle_statusbar,
        toggle_win_theme,
        toggle_cursor_blink,
        toggle_block_cursor,
        toggle_line_highlight,
        toggle_shadow,
        toggle_theme,
        toggle_keyfilter,
        toggle_line_numbers,
        toggle_scrolls,
        load_recent
        })
        action->setCheckable(true);
    loadMenuToggle(toggle_aot, Ud::ConfigGroup::Window, Ud::ConfigVal::T_AotBtn, false);
    loadMenuToggle(toggle_bar, Ud::ConfigGroup::Window, Ud::ConfigVal::T_ColorBar, true);
    loadMenuToggle(toggle_indicator, Ud::ConfigGroup::Window, Ud::ConfigVal::T_Indicator, false);
    loadMenuToggle(toggle_pane, Ud::ConfigGroup::Window, Ud::ConfigVal::T_Pane, true);
    loadMenuToggle(toggle_statusbar, Ud::ConfigGroup::Window, Ud::ConfigVal::T_StatusBar, true);
    loadMenuToggle(toggle_win_theme, Ud::ConfigGroup::Window, Ud::ConfigVal::T_WinTheme, true);
    loadMenuToggle(toggle_cursor_blink, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_CursorBlink, true);
    loadMenuToggle(toggle_block_cursor, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Cursor, true);
    loadMenuToggle(toggle_line_highlight, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_LineHighlight, true);
    loadMenuToggle(toggle_shadow, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Shadow, false);
    loadMenuToggle(toggle_theme, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_EditorTheme, true);
    loadMenuToggle(toggle_keyfilter, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Keyfilter, true);
    loadMenuToggle(toggle_line_numbers, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Lna, false);
    loadMenuToggle(toggle_scrolls, Ud::ConfigGroup::Editor, Ud::ConfigVal::T_Nav, true);
    loadMenuToggle(load_recent, Ud::ConfigGroup::Data, Ud::ConfigVal::T_Lmr, false);
    auto* toggle = menuBar->addMenu(tr("&Toggle"));
    toggle->addAction(toggle_aot);
    toggle->addAction(toggle_bar);
    toggle->addAction(toggle_indicator);
    toggle->addAction(toggle_pane);
    toggle->addAction(toggle_statusbar);
    toggle->addAction(toggle_win_theme);
    toggle->addSeparator();
    auto* cursor = toggle->addMenu(tr("&Cursor"));
    cursor->addAction(toggle_cursor_blink);
    cursor->addAction(toggle_block_cursor);
    toggle->addAction(toggle_shadow);
    toggle->addAction(toggle_theme);
    toggle->addAction(toggle_keyfilter);
    toggle->addAction(toggle_line_numbers);
    toggle->addAction(toggle_scrolls);
    toggle->addSeparator();
    toggle->addAction(load_recent);
}

void Fernanda::makeHelpMenu()
{
    auto* about = new QAction(tr("&About..."), this);
    auto* shortcuts = new QAction(tr("&Shortcuts..."), this);
    auto* open_docs = new QAction(tr("&Open documents..."), this);
    auto* open_ud = new QAction(tr("&Open user data..."), this);
    auto* sample_project = new QAction(tr("&Create sample project"), this);
    auto* sample_themes = new QAction(tr("&Create sample themes..."), this);
    connect(about, &QAction::triggered, this, &Fernanda::helpAbout);
    connect(shortcuts, &QAction::triggered, this, &Fernanda::helpShortcuts);
    connect(open_docs, &QAction::triggered, this, [&]() { openUd(Ud::userData(Ud::Op::GetDocs)); });
    connect(open_ud, &QAction::triggered, this, [&]() { openUd(Ud::userData(Ud::Op::GetUserData)); });
    connect(sample_project, &QAction::triggered, this, &Fernanda::helpMakeSampleProject);
    connect(sample_themes, &QAction::triggered, this, &Fernanda::helpMakeSampleRes);
    auto* help = menuBar->addMenu(tr("&Help"));
    help->addAction(about);
    help->addAction(shortcuts);
    help->addSeparator();
    help->addAction(open_docs);
    help->addAction(open_ud);
    help->addSeparator();
    help->addAction(sample_project);
    help->addAction(sample_themes);
}

void Fernanda::makeDevMenu()
{
    auto* print_cursors = new QAction(tr("&Print cursor positions"), this);
    auto* print_cuts = new QAction(tr("&Print cuts"), this);
    auto* print_dom = new QAction(tr("&Print DOM"), this);
    auto* print_edited_delegate = new QAction(tr("&Print edited keys (Delegate)"), this);
    auto* print_edited_story = new QAction(tr("&Print edited keys (Story)"), this);
    auto* print_renames = new QAction(tr("&Print renames"), this);
    auto* open_docs = new QAction(tr("&Open documents..."), this);
    auto* open_temps = new QAction(tr("&Open temps..."), this);
    auto* open_ud = new QAction(tr("&Open user data..."), this);
    connect(print_cursors, &QAction::triggered, this, [&]()
        {
            devWrite("__Cursor positions.txt", textEditor->devGetCursorPositions().join("\n\n"));
        });
    connect(print_cuts, &QAction::triggered, this, [&]()
        {
            if (!activeStory.has_value()) return;
            devWrite("__Cuts.xml", activeStory.value().devGetDom(Dom::Doc::Cuts));
        });
    connect(print_dom, &QAction::triggered, this, [&]()
        {
            if (!activeStory.has_value()) return;
            devWrite("__DOM.xml", activeStory.value().devGetDom());
        });
    connect(print_edited_delegate, &QAction::triggered, this, [&]()
        {
            devWrite("__Edited keys (Delegate).txt", pane->devGetEditedKeys().join("\n\n"));
        });
    connect(print_edited_story, &QAction::triggered, this, [&]()
        {
            if (!activeStory.has_value()) return;
            devWrite("__Edited keys (Story).txt", activeStory.value().devGetEditedKeys().join("\n\n"));
        });
    connect(print_renames, &QAction::triggered, this, [&]()
        {
            if (!activeStory.has_value()) return;
            auto renames = devPrintRenames(activeStory.value().devGetRenames());
            devWrite("__Renames.txt", renames.join("\n\n"));
        });
    connect(open_docs, &QAction::triggered, this, [&]() { openUd(Ud::userData(Ud::Op::GetDocs)); });
    connect(open_temps, &QAction::triggered, this, [&]()
        {
            if (!activeStory.has_value()) return;
            openUd(activeStory.value().devGetActiveTemp());
        });
    connect(open_ud, &QAction::triggered, this, [&]() { openUd(Ud::userData(Ud::Op::GetUserData)); });
    auto* dev = menuBar->addMenu(tr("&Dev"));
    dev->addAction(print_cursors);
    dev->addAction(print_cuts);
    dev->addAction(print_dom);
    dev->addAction(print_edited_delegate);
    dev->addAction(print_edited_story);
    dev->addAction(print_renames);
    dev->addSeparator();
    dev->addAction(open_docs);
    dev->addAction(open_temps);
    dev->addAction(open_ud);
}

QActionGroup* Fernanda::makeViewToggles(QVector<Res::DataPair>& dataLabelPairs, void (Fernanda::* slot)())
{
    auto* group = new QActionGroup(this);
    for (auto& pair : dataLabelPairs)
    {
        auto& data = pair.path;
        auto label = pair.label.toUtf8();
        auto* action = new QAction(tr(label), this);
        action->setData(Path::toQString(data));
        connect(action, &QAction::toggled, this, slot);
        action->setCheckable(true);
        group->addAction(action);
    }
    group->setExclusive(true);
    return group;
}

void Fernanda::loadConfigs(FsPath story)
{
    loadWinConfigs();
    auto value = Ud::loadConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::FontSize, 14, Ud::Type::Int).toInt();
    fontSlider->setValue(value);
    splitter->loadConfig(geometry());
    auto is_empty = story.empty();
    auto load_most_recent = Ud::loadConfig(Ud::ConfigGroup::Data, Ud::ConfigVal::T_Lmr, false, Ud::Type::Bool).toBool();
    if (load_most_recent || !is_empty)
        hasStartUpBar = false;
    if (!is_empty)
    {
        openStory(story);
        return;
    }
    if (load_most_recent)
    {
        auto project = Path::toFs(Ud::loadConfig(Ud::ConfigGroup::Data, Ud::ConfigVal::Project));
        if (!QFile(project).exists() || project.empty()) return;
        openStory(project);
    }
}

void Fernanda::loadWinConfigs()
{
    auto geometry = Ud::loadConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::Position, QRect(0, 0, 1000, 666), Ud::Type::QRect).toRect();
    setGeometry(geometry);
    auto win_state = Ud::loadConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::State).toInt();
    if (win_state == 1) setWindowState(Qt::WindowState::WindowMinimized);
    else if (win_state == 2) setWindowState(Qt::WindowState::WindowMaximized);
    else if (win_state == 4) setWindowState(Qt::WindowState::WindowFullScreen);
    auto checked = Ud::loadConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::Aot, false).toBool();
    aot->setChecked(checked);
}

void Fernanda::loadViewConfig(QVector<QAction*> actions, Ud::ConfigGroup group, Ud::ConfigVal valueType, QVariant fallback)
{
    auto resource = Ud::loadConfig(group, valueType, fallback);
    for (auto action : actions)
        if (Path::toFs(action->data()) == Path::toFs(resource))
        {
            action->setChecked(true);
            return;
        }
    for (auto action : actions)
        if (Path::toFs(action->data()) == Path::toFs(fallback))
        {
            action->setChecked(true);
            return;
        }
    actions.first()->setChecked(true);
}

void Fernanda::loadMenuToggle(QAction* action, Ud::ConfigGroup group, Ud::ConfigVal valueType, QVariant fallback)
{
    auto toggle_state = Ud::loadConfig(group, valueType, fallback, Ud::Type::Bool).toBool();
    action->setChecked(!toggle_state); // whyyyyyyyyy
    action->setChecked(toggle_state);
}

void Fernanda::openStory(FsPath fileName, Story::Op opt)
{
    if (fileName.empty())
    {
        colorBar->red();
        return;
    }
    auto change = confirmStoryClose();
    if (!change) return;
    Ud::clear(Ud::userData(Ud::Op::GetTemp));
    activeStory = Story(fileName, opt);
    auto& story = activeStory.value();
    addStoryToTitle(fileName);
    askEditorClose(true);
    sendItems(story.items());
    colorBar->green();
    Ud::saveConfig(Ud::ConfigGroup::Data, Ud::ConfigVal::Project, Path::toQString(fileName));
}

void Fernanda::actionCycle(QActionGroup* group)
{
    auto actions = group->actions();
    auto current_theme = group->checkedAction();
    if (current_theme != actions.last())
    {
        auto set_next = false;
        for (auto& action : actions)
        {
            if (set_next)
            {
                action->setChecked(true);
                break;
            }
            if (action == current_theme)
                set_next = true;
        }
    }
    else
        actions.first()->setChecked(true);
}

const QString Fernanda::windowStyle(WinStyle baseOnly)
{
    if (auto selection = windowThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = Path::toFs(selection->data());
        auto style_sheet = Io::readFile(":/themes/window_base.qss");
        if (hasWinTheme && baseOnly != WinStyle::BaseOnly)
        {
            auto theme_sheet = Io::readFile(theme_path);
            style_sheet = style_sheet + "\n\n" + Res::createStyleSheetFromTheme(Io::readFile(":/themes/window.qss"), theme_sheet);
        }
        sendScrollsToggle(hasWinTheme);
        Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::WinTheme, Path::toQString(theme_path));
        return style_sheet;
    }
    return nullptr;
}

void Fernanda::toggleGlobals(bool& globalBool, Ud::ConfigGroup group, Ud::ConfigVal valueType, bool value, Toggle type)
{
    globalBool = value;
    Ud::saveConfig(group, valueType, value);
    switch (type) {
    case Toggle::None:
        break;
    case Toggle::Count:
        textEditor->textChanged();
        break;
    case Toggle::Pos:
        textEditor->cursorPositionChanged();
        break;
    case Toggle::WinTheme:
        setWindowStyle();
        break;
    case Toggle::Theme:
        setEditorStyle();
        break;
    }
}

void Fernanda::toggleWidget(QWidget* widget, Ud::ConfigGroup group, Ud::ConfigVal valueType, bool value)
{
    widget->setVisible(value);
    Ud::saveConfig(group, valueType, value);
}

void Fernanda::setWindowStyle()
{
    setStyleSheet(windowStyle());
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = Path::toFs(selection->data());
        auto style_sheet = Io::readFile(":/themes/editor_base.qss");
        QString cursor_color = nullptr;
        QString under_cursor_color = nullptr;
        if (hasTheme)
        {
            auto theme_sheet = Io::readFile(theme_path);
            style_sheet = style_sheet + "\n\n" + Res::createStyleSheetFromTheme(Io::readFile(":/themes/editor.qss"), theme_sheet);
            QRegularExpressionMatch match_cursor = Uni::regex(Uni::Re::ThemeSheetCursor).match(theme_sheet);
            QRegularExpressionMatch match_under_cursor = Uni::regex(Uni::Re::ThemeSheetCursorUnder).match(theme_sheet);
            cursor_color = match_cursor.captured(2);
            under_cursor_color = match_under_cursor.captured(2);
        }
        if (hasShadow)
            style_sheet = style_sheet + "\n\n" + Io::readFile(":/themes/shadow.qss");
        overlay->setStyleSheet(style_sheet);
        underlay->setStyleSheet(style_sheet);
        textEditor->setStyleSheet(style_sheet);
        textEditor->cursorColorHex = cursor_color;
        textEditor->cursorUnderColorHex = under_cursor_color;
        Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::EditorTheme, Path::toQString(theme_path));
    }
}

void Fernanda::setEditorFont()
{
    if (auto selection = editorFonts->checkedAction(); selection != nullptr)
    {
        auto font = selection->data().toString();
        textEditor->setFont(font, fontSlider->value());
        Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::Font, font);
    }
}

void Fernanda::handleEditorZoom(TextEditor::Zoom direction)
{
    switch (direction) {
    case TextEditor::Zoom::In:
        fontSlider->setValue(fontSlider->value() + 2);
        break;
    case TextEditor::Zoom::Out:
        fontSlider->setValue(fontSlider->value() - 2);
        break;
    }
}

void Fernanda::aotToggled(bool checked)
{
    if (checked)
    {
        setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        aot->setText(Uni::ico(Uni::Ico::Balloon));
    }
    else
    {
        setWindowFlags(windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        aot->setText(Uni::ico(Uni::Ico::Pushpin));
    }
    show();
    Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::Aot, checked);
}

void Fernanda::setTabStop()
{
    if (auto selection = tabStops->checkedAction(); selection != nullptr)
    {
        auto distance = selection->data().toInt();
        textEditor->setTabStopDistance(distance);
        Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::TabStop, distance);
    }
}

void Fernanda::setWrapMode()
{
    if (auto selection = wrapModes->checkedAction(); selection != nullptr)
    {
        auto mode = selection->data().toString();
        if (mode == "NoWrap")
            textEditor->setWordWrapMode(QTextOption::NoWrap);
        else if (mode == "WordWrap")
            textEditor->setWordWrapMode(QTextOption::WordWrap);
        else if (mode == "WrapAnywhere")
            textEditor->setWordWrapMode(QTextOption::WrapAnywhere);
        else if (mode == "WrapAt")
            textEditor->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        Ud::saveConfig(Ud::ConfigGroup::Editor, Ud::ConfigVal::Wrap, mode);
    }
}

void Fernanda::setBarAlignment()
{
    if (auto selection = barAlignments->checkedAction(); selection != nullptr)
    {
        auto alignment = selection->data().toString();
        (alignment == "Bottom")
            ? colorBar->align(Qt::AlignBottom)
            : colorBar->align(Qt::AlignTop);
        Ud::saveConfig(Ud::ConfigGroup::Window, Ud::ConfigVal::BarAlign, alignment);
    }
}

void Fernanda::fileSave()
{
    if (!activeStory.has_value()) return;
    auto& story = activeStory.value();
    if (!story.hasChanges()) return;
    story.save(textEditor->toPlainText());
    Ud::clear(Ud::userData(Ud::Op::GetTemp));
    textEditor->textChanged();
    sendItems(story.items());
    colorBar->green();
}

void Fernanda::helpMakeSampleProject()
{
    auto path = Ud::userData(Ud::Op::GetDocs) / "Candide.story";
    openStory(path, Story::Op::Sample);
}

void Fernanda::helpMakeSampleRes()
{
    auto path = Ud::userData(Ud::Op::GetUserData);
    Sample::makeRc(path);
    colorBar->pastels();
    QMessageBox alert;
    alert.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    alert.setWindowTitle("Hey!");
    alert.setText(Uni::samples());
    auto ok = alert.addButton(QMessageBox::Ok);
    auto open = alert.addButton(tr("Open the user data folder"), QMessageBox::AcceptRole);
    alert.setDefaultButton(ok);
    alert.exec();
    if (alert.clickedButton() == open)
        openUd(path);
}

void Fernanda::helpShortcuts()
{
    QMessageBox shortcuts;
    shortcuts.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    shortcuts.setWindowTitle("Shortcuts");
    shortcuts.setText(Uni::shortcuts());
    auto ok = shortcuts.addButton(QMessageBox::Ok);
    shortcuts.setDefaultButton(ok);
    shortcuts.exec();
}

void Fernanda::helpAbout()
{
    QMessageBox about;
    about.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    about.setWindowTitle("About");
    about.setText(Uni::about());
    auto ok = about.addButton(QMessageBox::Ok);
    auto qt = about.addButton(tr("About Qt"), QMessageBox::AcceptRole);
    connect(qt, &QPushButton::clicked, this, QApplication::aboutQt);
    about.setDefaultButton(ok);
    about.exec();
}

void Fernanda::devWrite(QString name, QString value)
{
    auto docs = Ud::userData(Ud::Op::GetDocs);
    Io::writeFile(docs / name.toStdString(), value);
}

void Fernanda::handleEditorOpen(QString key)
{
    QString old_key = nullptr;
    if (activeStory.has_value())
        old_key = activeStory.value().key();
    auto action = textEditor->handleKeySwap(old_key, key);
    switch (action) {
    case TextEditor::Action::None: break;
    case TextEditor::Action::AcceptNew:
        {
            auto text = activeStory.value().tempSaveOld_openNew(key, textEditor->toPlainText());
            textEditor->handleTextSwap(key, text);
            startAutoTempSave();
        }
        break;
    }
}

void Fernanda::sendEditedText()
{
    if (!activeStory.has_value()) return;
    sendEditsList(activeStory.value().edits(textEditor->toPlainText()));
}

bool Fernanda::replyHasProject()
{
    if (activeStory.has_value()) return true;
    return false;
}

void Fernanda::domMove(QString pivotKey, QString fulcrumKey, Io::Move pos)
{
    auto& story = activeStory.value();
    story.move(pivotKey, fulcrumKey, pos);
    sendItems(story.items());
}

void Fernanda::domAdd(QString newName, Path::Type type, QString parentKey)
{
    auto& story = activeStory.value();
    story.add(newName, type, parentKey);
    sendItems(story.items());
}

void Fernanda::domRename(QString newName, QString key)
{
    auto& story = activeStory.value();
    story.rename(newName, key);
    sendItems(story.items());
}

void Fernanda::domCut(QString key)
{
    auto& story = activeStory.value();
    auto active_key_cut = story.cut(key);
    sendItems(story.items());
    if (!active_key_cut)
    {
        textEditor->setFocus();
        return;
    }
    askEditorClose();
}

void Fernanda::cycleCoreEditorThemes()
{

    auto actions = editorThemes->actions();
    auto current_theme = editorThemes->checkedAction();
    auto text = current_theme->text();
    auto break_it = false;
    auto theme_1 = "Amber";
    auto theme_2 = "Green";
    for (auto& action : actions)
    {
        auto action_text = action->text();
        if (text != theme_1 && text != theme_2 && action_text == theme_1)
            break_it = true;
        else if (text == theme_1 && action_text == theme_2)
            break_it = true;
        else if (text == theme_2 && action_text == "Grey")
            break_it = true;
        if (break_it)
        {
            action->setChecked(true);
            break;
        }
    }
}

void Fernanda::triggerOverlay(TextEditor::Overlay state)
{
    switch (state) {
    case TextEditor::Overlay::Hide:
        if (overlay->isVisible())
            overlay->hide();
        break;
    case TextEditor::Overlay::Show:
        if (!overlay->isVisible())
            overlay->show();
        break;
    }
}

// fernanda.cpp, fernanda