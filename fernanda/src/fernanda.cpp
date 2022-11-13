// fernanda.cpp, fernanda

#include "fernanda.h"

Fernanda::Fernanda(QWidget* parent)
    : QMainWindow(parent)
{
    Ud::windowsReg();
    addWidgets();
    connections();
    Ud::userData(Ud::Op::Create, ferName);
    makeMenuBar();
    loadConfigs();
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
    Ud::saveConfig("window", "position", geometry());
}

void Fernanda::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    Ud::saveConfig("window", "position", geometry());
}

void Fernanda::closeEvent(QCloseEvent* event)
{
    auto state = windowState();
    Ud::saveConfig("window", "state", state.toInt());
    setWindowState(Qt::WindowState::WindowActive);
    if (activeProject.has_value() && activeProject.value().hasAnyChanges())
    {
        QMessageBox alert;
        alert.setWindowTitle("Hey!");
        alert.setText("You have unsaved changes. Are you sure you want to quit?");
        alert.addButton(QMessageBox::Yes);
        auto no = alert.addButton(QMessageBox::No);
        alert.setDefaultButton(no);
        alert.exec();
        if (alert.clickedButton() == no)
        {
            setWindowState(state);
            event->ignore();
            colorBar->green();
            return;
        }
    }
    Ud::clear(Ud::userData(Ud::Op::GetTemp), true);
    event->accept();
}

void Fernanda::addWidgets()
{
    QWidget* main_container = new QWidget(this);
    QWidget* editor_container = new QWidget(this);
    QWidget* bar_container = new QWidget(this);
    QStackedLayout* main_stack = new QStackedLayout(main_container);
    QStackedLayout* editor_stack = new QStackedLayout(editor_container);
    barLayout = new QVBoxLayout(bar_container);
    barLayout->addWidget(colorBar);
    main_stack->addWidget(bar_container);
    main_stack->addWidget(splitter);
    editor_stack->addWidget(overlay);
    editor_stack->addWidget(textEditor);
    editor_stack->addWidget(underlay);
    splitter->addWidget(pane);
    splitter->addWidget(editor_container);

    statusBar->setSizeGripEnabled(true);
    main_stack->setStackingMode(QStackedLayout::StackAll);
    editor_stack->setStackingMode(QStackedLayout::StackAll);
    barLayout->setContentsMargins(0, 0, 0, 0);
    bar_container->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    underlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    splitter->setCollapsible(0, true);
    splitter->setCollapsible(1, false);
    splitter->setStretchFactor(1, 100);

    setWindowTitle(ferName);
    setMenuBar(menuBar);
    setStatusBar(statusBar);
    setCentralWidget(main_container);

    aot->setCheckable(true);
    aot->setText(Uni::ico.pushpin);

    statusBar->addPermanentWidget(positions, 0);
    statusBar->addPermanentWidget(counters, 0);
    statusBar->addPermanentWidget(spacer, 1);
    statusBar->addPermanentWidget(aot, 0);
    statusBar->setMaximumHeight(22);

    menuBar->setObjectName("menuBar");
    statusBar->setObjectName("statusBar");
    splitter->setObjectName("splitter");
    overlay->setObjectName("overlay");
    underlay->setObjectName("underlay");
    fontSlider->setObjectName("fontSlider");
    positions->setObjectName("positions");
    counters->setObjectName("counters");
    spacer->setObjectName("spacer");
    aot->setObjectName("aot");
}

void Fernanda::connections()
{
    connect(textEditor, &TextEditor::askNavPrevious, pane, [&]() { pane->nav(Pane::Nav::Prev); });
    connect(textEditor, &TextEditor::askNavNext, pane, [&]() { pane->nav(Pane::Nav::Next); });
    connect(splitter, &QSplitter::splitterMoved, this, [&]()
        {
            Ud::saveConfig("window", "splitter", splitter->saveState());
        });
    connect(this, &Fernanda::startAutoTempSave, this, [&]() { autoTempSave->start(30000); });
    connect(autoTempSave, &QTimer::timeout, this, [&]()
        {
            if (!activeProject.has_value()) return;
            activeProject.value().autoTempSave(textEditor->toPlainText());
        });
    connect(pane, &Pane::askDomMove, this, &Fernanda::domMove);
    connect(pane, &Pane::askAddElement, this, &Fernanda::addElement);
    connect(pane, &Pane::askRenameElement, this, &Fernanda::renameElement);
    connect(textEditor, &TextEditor::askFontSliderZoom, this, &Fernanda::handleEditorZoom);
    connect(pane, &Pane::askHasProject, this, &Fernanda::replyHasProject);
    connect(textEditor, &TextEditor::askHasProject, this, &Fernanda::replyHasProject);
    connect(this, &Fernanda::sendColorBarToggle, colorBar, &ColorBar::toggleSelf);
    connect(this, &Fernanda::sendLineHighlightToggle, textEditor, &TextEditor::toggleLineHighlight);
    connect(this, &Fernanda::sendLineNumberAreaToggle, textEditor, &TextEditor::toggleLineNumberArea);
    connect(this, &Fernanda::sendScrollsToggle, textEditor, &TextEditor::toggleScrolls);
    connect(this, &Fernanda::sendExtraScrollsToggle, textEditor, &TextEditor::toggleExtraScrolls);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
    connect(textEditor, &TextEditor::cursorPositionChanged, this, &Fernanda::updatePositions);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::updateCounters);
    connect(this, &Fernanda::sendInitExpansions, pane, &Pane::receiveInitExpansions);
    connect(this, &Fernanda::sendItems, pane, &Pane::receiveItems);
    connect(pane, &Pane::askSendToEditor, this, &Fernanda::handleEditorText);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::sendEditedText);
    connect(this, &Fernanda::sendEditsList, pane, &Pane::receiveEditsList);
    connect(this, &Fernanda::saveCursors, textEditor, &TextEditor::storeCursors);
    connect(this, &Fernanda::applyCursors, textEditor, &TextEditor::recallCursors);
    connect(this, &Fernanda::saveUndoStacks, textEditor, &TextEditor::storeUndoStacks);
    connect(this, &Fernanda::applyUndoStacks, textEditor, &TextEditor::recallUndoStacks);
}

void Fernanda::makeMenuBar()
{
    makeFileMenu();
    makeViewMenu();
    makeHelpMenu();
}

void Fernanda::makeFileMenu()
{
    auto* file = menuBar->addMenu(tr("&File"));
    auto* new_project = new QAction(tr("&New project..."), this);
    auto* open_project = new QAction(tr("&Open an existing project..."), this);
    auto* save = new QAction(tr("&Save"), this);
    auto* quit = new QAction(tr("&Quit"), this);
    save->setShortcut(Qt::CTRL | Qt::Key_S);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(new_project, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new project"), Ud::userData(Ud::Op::GetDocs), tr("Fernanda project file (*.story)"));
            openProject(file_name);
        });
    connect(open_project, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getOpenFileName(this, tr("Open an existing project"), Ud::userData(Ud::Op::GetDocs), tr("Fernanda project file (*.story)"));
            openProject(file_name);
        });
    connect(save, &QAction::triggered, this, &Fernanda::fileSave);
    connect(quit, &QAction::triggered, this, &QCoreApplication::quit, Qt::QueuedConnection);
    file->addAction(new_project);
    file->addAction(open_project);
    file->addSeparator();
    file->addAction(save);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeViewMenu() // clean me
{
    auto user_data = Ud::userData(Ud::Op::GetUserData);
    auto win_theme_list = Res::iterateResources(":\\themes\\window\\", "*.fernanda_wintheme", user_data, Res::Type::WindowTheme);
    windowThemes = makeViewToggles(win_theme_list, &Fernanda::setWindowStyle);
    loadViewConfig(windowThemes->actions(), "window", "wintheme", ":\\themes\\window\\light.fernanda_wintheme");
    auto editor_theme_list = Res::iterateResources(":\\themes\\editor\\", "*.fernanda_theme", user_data, Res::Type::EditorTheme);
    editorThemes = makeViewToggles(editor_theme_list, &Fernanda::setEditorStyle);
    loadViewConfig(editorThemes->actions(), "editor", "theme", ":\\themes\\editor\\amber.fernanda_theme");
    auto font_list = Res::iterateResources(":\\fonts\\", "*.ttf", user_data, Res::Type::Font);
    editorFonts = makeViewToggles(font_list, &Fernanda::setEditorFont);
    loadViewConfig(editorFonts->actions(), "editor", "font", "Cascadia Code");

    auto* toggle_win_theme = new QAction(tr("&Toggle window theme"), this);
    connect(toggle_win_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasWinTheme, "window", "wintheme_on", checked, Toggle::WinTheme);
        });
    toggle_win_theme->setCheckable(true);
    loadMenuToggle(toggle_win_theme, "window", "wintheme_on", true);

    auto* toggle_theme = new QAction(tr("&Toggle editor theme"), this);
    connect(toggle_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasTheme, "editor", "theme_on", checked, Toggle::Theme);
        });
    toggle_theme->setCheckable(true);
    loadMenuToggle(toggle_theme, "editor", "theme_on", true);

    auto* font_size_label = new QAction(tr("&Set editor font size:"), this);
    font_size_label->setEnabled(false);
    auto* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(64);
    connect(fontSlider, &QSlider::valueChanged, this, [&](int value)
        {
            setEditorFont();
            Ud::saveConfig("editor", "font_size", value);
        });

    QVector<Res::DataPair> tab_list = {
        Res::DataPair{ "20", "20 px" },
        Res::DataPair{ "40", "40 px" },
        Res::DataPair{ "60", "60 px" },
        Res::DataPair{ "80", "80 px" }
    };
    tabStops = makeViewToggles(tab_list, &Fernanda::setTabStop);
    loadViewConfig(tabStops->actions(), "editor", "tab", "40");

    QVector<Res::DataPair> wrap_list = {
        Res::DataPair{ "NoWrap", "No wrap" },
        Res::DataPair{ "WordWrap", "Wrap at word boundaries" },
        Res::DataPair{ "WrapAnywhere", "Wrap anywhere" },
        Res::DataPair{ "WrapAt", "Wrap at word boundaries or anywhere" }
    };
    wrapModes = makeViewToggles(wrap_list, &Fernanda::setWrapMode);
    loadViewConfig(wrapModes->actions(), "editor", "wrap", "WrapAt");

    QVector<Res::DataPair> bar_alignments = {
        Res::DataPair{ "Top", "Top" },
        Res::DataPair{ "Bottom", "Bottom" }
    };
    barAlignments = makeViewToggles(bar_alignments, &Fernanda::setBarAlignment);
    loadViewConfig(barAlignments->actions(), "window", "bar_alignment", "Top");

    auto* toggle_aot = new QAction(tr("&Toggle always-on-top button"), this);
    connect(toggle_aot, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(aot, "window", "aot_button", checked);
        });
    toggle_aot->setCheckable(true);
    loadMenuToggle(toggle_aot, "window", "aot_button", false);

    auto* toggle_bar = new QAction(tr("&Toggle color bar"), this);
    connect(toggle_bar, &QAction::toggled, this, [&](bool checked)
        {
            sendColorBarToggle(checked);
            Ud::saveConfig("window", "color_bar", checked);
        });
    toggle_bar->setCheckable(true);
    loadMenuToggle(toggle_bar, "window", "color_bar", true);

    auto* toggle_pane = new QAction(tr("&Toggle file pane"), this);
    connect(toggle_pane, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(pane, "window", "pane", checked);
        });
    toggle_pane->setCheckable(true);
    loadMenuToggle(toggle_pane, "window", "pane", true);

    auto* toggle_statusbar = new QAction(tr("&Toggle statusbar"), this);
    connect(toggle_statusbar, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(statusBar, "window", "statusbar", checked);
        });
    toggle_statusbar->setCheckable(true);
    loadMenuToggle(toggle_statusbar, "window", "statusbar", true);

    auto* toggle_line_highlight = new QAction(tr("&Toggle current line highlight"), this);
    connect(toggle_line_highlight, &QAction::toggled, this, [&](bool checked)
        {
            sendLineHighlightToggle(checked);
            Ud::saveConfig("editor", "line_highlight", checked);
        });
    toggle_line_highlight->setCheckable(true);
    loadMenuToggle(toggle_line_highlight, "editor", "line_highlight", true);

    auto* toggle_line_numbers = new QAction(tr("&Toggle line number area"), this);
    connect(toggle_line_numbers, &QAction::toggled, this, [&](bool checked)
        {
            sendLineNumberAreaToggle(checked);
            Ud::saveConfig("editor", "line_number_area", checked);
        });
    toggle_line_numbers->setCheckable(true);
    loadMenuToggle(toggle_line_numbers, "editor", "line_number_area", false);

    auto* toggle_scrolls = new QAction(tr("&Toggle scrolls previous and next"), this);
    connect(toggle_scrolls, &QAction::toggled, this, [&](bool checked)
        {
            sendExtraScrollsToggle(checked);
            Ud::saveConfig("editor", "nav_scrolls", checked);
        });
    toggle_scrolls->setCheckable(true);
    loadMenuToggle(toggle_scrolls, "editor", "nav_scrolls", true);

    auto* toggle_line_pos = new QAction(tr("&Toggle line position"), this);
    connect(toggle_line_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasLinePos, "window", "position_line", checked, Toggle::Pos);
        });
    toggle_line_pos->setCheckable(true);
    loadMenuToggle(toggle_line_pos, "window", "position_line", false);

    auto* toggle_col_pos = new QAction(tr("&Toggle column position"), this);
    connect(toggle_col_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasColPos, "window", "position_column", checked, Toggle::Pos);
        });
    toggle_col_pos->setCheckable(true);
    loadMenuToggle(toggle_col_pos, "window", "position_column", false);

    auto* toggle_line_count = new QAction(tr("&Toggle line count"), this);
    connect(toggle_line_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasLineCount, "window", "count_lines", checked, Toggle::Count);
        });
    toggle_line_count->setCheckable(true);
    loadMenuToggle(toggle_line_count, "window", "count_lines", false);

    auto* toggle_word_count = new QAction(tr("&Toggle word count"), this);
    connect(toggle_word_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasWordCount, "window", "count_words", checked, Toggle::Count);
        });
    toggle_word_count->setCheckable(true);
    loadMenuToggle(toggle_word_count, "window", "count_words", false);

    auto* toggle_char_count = new QAction(tr("&Toggle character count"), this);
    connect(toggle_char_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasCharCount, "window", "count_characters", checked, Toggle::Count);
        });
    toggle_char_count->setCheckable(true);
    loadMenuToggle(toggle_char_count, "window", "count_characters", false);

    auto* view = menuBar->addMenu(tr("&View"));
    auto* window_themes = view->addMenu(tr("&Set window theme"));
    window_themes->addActions(windowThemes->actions());
    auto* editor_themes = view->addMenu(tr("&Set editor theme"));
    editor_themes->addActions(editorThemes->actions());
    view->addAction(toggle_win_theme);
    view->addAction(toggle_theme);
    view->addSeparator();
    auto* fonts = view->addMenu(tr("&Set editor font"));
    fonts->addActions(editorFonts->actions());
    view->addAction(font_size_label);
    view->addAction(font_size);
    view->addSeparator();
    auto* tab_stops = view->addMenu(tr("&Set editor tab stop distance"));
    tab_stops->addActions(tabStops->actions());
    auto* wrap_modes = view->addMenu(tr("&Set editor wrap mode"));
    wrap_modes->addActions(wrapModes->actions());
    view->addSeparator();
    auto* bar_alignment = view->addMenu(tr("&Set color bar alignment"));
    bar_alignment->addActions(barAlignments->actions());
    view->addSeparator();
    view->addAction(toggle_aot);
    view->addAction(toggle_bar);
    view->addAction(toggle_pane);
    view->addAction(toggle_statusbar);
    view->addSeparator();
    view->addAction(toggle_line_highlight);
    view->addAction(toggle_line_numbers);
    view->addAction(toggle_scrolls);
    view->addSeparator();
    auto* positions = view->addMenu(tr("&Show line and column positions"));
    positions->addAction(toggle_line_pos);
    positions->addAction(toggle_col_pos);
    auto* counters = view->addMenu(tr("&Show line, word, and character counts"));
    counters->addAction(toggle_line_count);
    counters->addAction(toggle_word_count);
    counters->addAction(toggle_char_count);
}

void Fernanda::makeHelpMenu()
{
    auto* help = menuBar->addMenu(tr("&Help"));
    //auto* projects_dir = new QAction(tr("&Set default projects directory..."), this);
    auto* load_recent = new QAction(tr("&Load most recent project on open"), this);
    auto* sample_project = new QAction(tr("&Generate sample project"), this);
    auto* sample_themes = new QAction(tr("&Generate sample themes"), this);
    auto* about = new QAction(tr("&About Fernanda"), this);
    //connect(projects_dir, &QAction::triggered, this, &Fernanda::helpProjects);
    load_recent->setCheckable(true);
    connect(load_recent, &QAction::toggled, this, [&](bool checked)
        {
            Ud::saveConfig("data", "load_most_recent", checked);
        });
    loadMenuToggle(load_recent, "data", "load_most_recent", false);
    connect(sample_project, &QAction::triggered, this, &Fernanda::helpMakeSampleProject);
    connect(sample_themes, &QAction::triggered, this, &Fernanda::helpMakeSampleRes);
    connect(about, &QAction::triggered, this, &Fernanda::helpAbout);
    //help->addAction(projects_dir);
    help->addAction(load_recent);
    help->addSeparator();
    help->addAction(sample_project);
    help->addAction(sample_themes);
    help->addSeparator();
    help->addAction(about);
}

QActionGroup* Fernanda::makeViewToggles(QVector<Res::DataPair>& dataLabelPairs, void (Fernanda::* slot)())
{
    auto* group = new QActionGroup(this);
    for (auto& pair : dataLabelPairs)
    {
        auto& data = pair.path;
        auto label = pair.label.toUtf8();
        auto* action = new QAction(tr(label), this);
        action->setData(data);
        connect(action, &QAction::toggled, this, slot);
        action->setCheckable(true);
        group->addAction(action);
    }
    group->setExclusive(true);
    return group;
}

void Fernanda::loadConfigs()
{
    loadWinConfigs();
    auto value = Ud::loadConfig("editor", "font_size", 14, Ud::Type::Int).toInt();
    fontSlider->setValue(value);
    auto state = Ud::loadConfig("window", "splitter").toByteArray();
    splitter->restoreState(state);
    auto has_project = Ud::loadConfig("data", "load_most_recent", false, Ud::Type::Bool).toBool();
    if (has_project)
    {
        auto project = Ud::loadConfig("data", "project").toString();
        if (!Path::exists(project) || project.isEmpty()) return;
        openProject(project);
        hasStartUpBar = false;
    }
}

void Fernanda::loadWinConfigs()
{
    auto geometry = Ud::loadConfig("window", "position", QRect(0, 0, 1000, 666), Ud::Type::QRect).toRect();
    setGeometry(geometry);
    auto win_state = Ud::loadConfig("window", "state").toInt();
    if (win_state == 1) setWindowState(Qt::WindowState::WindowMinimized);
    else if (win_state == 2) setWindowState(Qt::WindowState::WindowMaximized);
    else if (win_state == 4) setWindowState(Qt::WindowState::WindowFullScreen);
    auto checked = Ud::loadConfig("window", "always_on_top", false).toBool();
    aot->setChecked(checked);
}

void Fernanda::loadViewConfig(QVector<QAction*> actions, QString group, QString valueName, QVariant fallback)
{
    auto resource = Ud::loadConfig(group, valueName, fallback);
    for (auto action : actions)
        if (action->data() == resource)
        {
            action->setChecked(true);
            return;
        }
    for (auto action : actions)
        if (action->data() == fallback)
        {
            action->setChecked(true);
            return;
        }
    actions.first()->setChecked(true);
}

void Fernanda::loadMenuToggle(QAction* action, QString group, QString valueName, QVariant fallback)
{
    auto toggle_state = Ud::loadConfig(group, valueName, fallback, Ud::Type::Bool).toBool();
    action->setChecked(!toggle_state); // whyyyyyyyyy
    action->setChecked(toggle_state);
}

void Fernanda::openProject(QString fileName, Project::SP opt)
{
    if (fileName.isEmpty())
    {
        colorBar->red();
        return;
    }
    activeProject = Project(fileName, opt);
    auto& project = activeProject.value();
    sendInitExpansions(project.makeInitExpansions());
    sendItems(project.makeItems());
    Ud::saveConfig("data", "project", fileName);
    colorBar->green();
}

void Fernanda::setWindowStyle()
{
    if (auto selection = windowThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = Res::createStyleSheetFromTheme(Io::readFile(":\\themes\\window.qss"), Io::readFile(theme_path));
        if (!hasWinTheme)
            style_sheet = nullptr;
        setStyleSheet(style_sheet);
        sendScrollsToggle(hasWinTheme);
        Ud::saveConfig("window", "wintheme", theme_path);
    }
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = Res::createStyleSheetFromTheme(Io::readFile(":\\themes\\editor.qss"), Io::readFile(theme_path));
        auto theme_cursor = Io::readFile(theme_path);
        QRegularExpressionMatch match_cursor = QRegularExpression(Uni::regex.themeSheetCursor).match(theme_cursor);
        QString cursor_color = match_cursor.captured(2);
        if (!hasTheme)
        {
            style_sheet = nullptr;
            cursor_color = nullptr;
        }
        overlay->setStyleSheet(style_sheet);
        textEditor->setStyleSheet(style_sheet);
        underlay->setStyleSheet(style_sheet);
        textEditor->cursorColorHex = cursor_color;
        Ud::saveConfig("editor", "theme", theme_path);
    }
}

void Fernanda::setEditorFont()
{
    if (auto selection = editorFonts->checkedAction(); selection != nullptr)
    {
        auto size = fontSlider->value();
        QFont font(selection->data().toString());
        font.setStyleStrategy(QFont::PreferAntialias);
        font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
        font.setPointSize(size);
        textEditor->setFont(font);
        textEditor->setCursorWidth(size * 0.8);
        textEditor->lineNumberAreaFont(size, selection->data().toString());
        Ud::saveConfig("editor", "font", selection->data().toString());
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

void Fernanda::toggleWidget(QWidget* widget, QString group, QString valueName, bool value)
{
    widget->setVisible(value);
    Ud::saveConfig(group, valueName, value);
}

void Fernanda::aotToggled(bool checked)
{
    if (checked)
    {
        setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        aot->setText(Uni::ico.balloon);
    }
    else
    {
        setWindowFlags(windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        aot->setText(Uni::ico.pushpin);
    }
    show();
    Ud::saveConfig("window", "always_on_top", checked);
}

void Fernanda::setTabStop()
{
    if (auto selection = tabStops->checkedAction(); selection != nullptr)
    {
        auto distance = selection->data().toInt();
        textEditor->setTabStopDistance(distance);
        Ud::saveConfig("editor", "tab", distance);
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
        Ud::saveConfig("editor", "wrap", mode);
    }
}

void Fernanda::setBarAlignment()
{
    if (auto selection = barAlignments->checkedAction(); selection != nullptr)
    {
        auto alignment = selection->data().toString();
        if (alignment == "Top")
            barLayout->setAlignment(Qt::AlignTop);
        else if (alignment == "Bottom")
            barLayout->setAlignment(Qt::AlignBottom);
        Ud::saveConfig("window", "bar_alignment", alignment);
    }
}

void Fernanda::toggleGlobals(bool& globalBool, QString group, QString valueName, bool value, Toggle type)
{
    globalBool = value;
    Ud::saveConfig(group, valueName, value);
    switch (type) {
    case Toggle::None: break;
    case Toggle::Count: updateCounters(); break;
    case Toggle::Pos: updatePositions(); break;
    case Toggle::WinTheme: setWindowStyle(); break;
    case Toggle::Theme: setEditorStyle(); break;
    }
}

void Fernanda::updatePositions()
{
    const auto line_pos = textEditor->textCursor().blockNumber();
    const auto col_pos = textEditor->textCursor().positionInBlock();
    QStringList elements;
    if (hasLinePos)
        elements << "ln " + QString::number(line_pos + 1);
    if (hasColPos)
        elements << "col " + QString::number(col_pos + 1);
    auto pos_display = elements.join(", ");
    positions->setText(pos_display);
}

void Fernanda::updateCounters()
{
    const auto text = textEditor->toPlainText();
    const auto line_count = textEditor->blockCount();
    const auto word_count = text.split(QRegularExpression(Uni::regex.splitCount)).count();
    const auto char_count = text.count();
    QStringList elements;
    if (hasLineCount)
        elements << QString::number(line_count) + " lines";
    if (hasWordCount)
        elements << QString::number(word_count - 1) + " words"; // - 1, whyyyyy
    // The above always displays at least 1, even when editor is blank?
    if (hasCharCount)
        elements << QString::number(char_count) + " chars";
    auto counter_display = elements.join(", ");
    counters->setText(counter_display);
}

void Fernanda::fileSave()
{
    if (!activeProject.has_value()) return;
    auto& project = activeProject.value();
    if (!project.hasAnyChanges()) return;
    project.saveProject(textEditor->toPlainText());
    textEditor->textChanged();
    sendItems(project.makeItems());
    colorBar->green();
}

/*void Fernanda::helpProjects()
{
    //
}*/

void Fernanda::helpMakeSampleProject()
{
    // ask to save if another project is open
    auto path = Ud::userData(Ud::Op::GetDocs) / "Candide.story";
    openProject(path, Project::SP::MakeSample);
}

void Fernanda::helpMakeSampleRes()
{
    auto path = Ud::userData(Ud::Op::GetUserData);
    Sample::makeRc(path.toStdString());
    // popup to tell user to restart to see changes
}

void Fernanda::helpAbout()
{
    QMessageBox about;
    about.setWindowTitle("About");
    about.setText("Fernanda is a personal project and a work-in-progress.");
    QAbstractButton* ok = about.addButton(tr("Okay"), QMessageBox::AcceptRole);
    about.exec();
}

void Fernanda::handleEditorText(QString key)
{
    if (!activeProject.has_value()) return;
    if (overlay->isVisible())
        overlay->hide();
    if (textEditor->isReadOnly())
        textEditor->setReadOnly(false);
    auto& project = activeProject.value();
    auto old_key = project.getActiveKey();
    if (old_key == key) // can we do this in project?
    {
        textEditor->setFocus();
        return;
    }
    if (old_key != nullptr) // can we do this in project?
    {
        saveCursors(old_key);
        saveUndoStacks(old_key);
    }
    auto text = project.saveOld_openNew(key, textEditor->toPlainText());
    textEditor->clear();
    textEditor->setPlainText(text);
    applyCursors(key);
    applyUndoStacks(key);
    textEditor->setFocus();
    startAutoTempSave();
}

void Fernanda::sendEditedText()
{
    if (!activeProject.has_value()) return;
    auto list = activeProject.value().handleEditsList(textEditor->toPlainText());
    sendEditsList(list);
}

bool Fernanda::replyHasProject()
{
    if (activeProject.has_value()) return true;
    else return false;
}

void Fernanda::domMove(QString pivotKey, QString fulcrumKey, Io::Move pos)
{
    if (!activeProject.has_value()) return;
    auto& project = activeProject.value();
    project.domMove(pivotKey, fulcrumKey, pos);
    sendItems(project.makeItems());
}

void Fernanda::addElement(QString newName, Path::Type type, QString parentKey)
{
    if (!activeProject.has_value()) return;
    auto& project = activeProject.value();
    project.newDomElement(newName, type, parentKey);
    sendItems(project.makeItems());
}

void Fernanda::renameElement(QString newName, QString key)
{
    if (!activeProject.has_value()) return;
    auto& project = activeProject.value();
    project.renameDomElement(newName, key);
    sendItems(project.makeItems());
}

// fernanda.cpp, fernanda