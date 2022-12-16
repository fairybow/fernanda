// fernanda.cpp, fernanda

#include "fernanda.h"

Fernanda::Fernanda(QWidget* parent)
    : QMainWindow(parent)
{
    setName();
    //Ud::windowsReg(); // WIP
    addWidgets();
    connections();
    Ud::userData();
    if (isDev) dev();
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
    if (activeStory.has_value() && activeStory.value().hasChanges())
    {
        QMessageBox alert;
        alert.setStyleSheet(windowStyle(WinStyle::BaseOnly));
        alert.setWindowTitle("Hey!");
        alert.setText(Uni::close());
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

void Fernanda::dev()
{
    auto print_dom = new QPushButton(this);
    statusBar->addPermanentWidget(print_dom, 0);
    print_dom->setText("Print DOM");
    print_dom->setObjectName("devPrint");
    connect(print_dom, &QPushButton::clicked, this, [&]()
        {
            if (!activeStory.has_value()) return;
            Io::writeFile((Ud::userData(Ud::Op::GetDocs) / "__DOM.xml"), activeStory.value().devGetDom());
        });
    auto print_cuts = new QPushButton(this);
    statusBar->addPermanentWidget(print_cuts, 0);
    print_cuts->setText("Print Cuts");
    print_cuts->setObjectName("devPrint");
    connect(print_cuts, &QPushButton::clicked, this, [&]()
        {
            if (!activeStory.has_value()) return;
            Io::writeFile((Ud::userData(Ud::Op::GetDocs) / "__Cuts.xml"), activeStory.value().devGetDom(Dom::Doc::Cuts));
        });
    auto print_renames = new QPushButton(this);
    statusBar->addPermanentWidget(print_renames, 0);
    print_renames->setText("Print Renames");
    print_renames->setObjectName("devPrint");
    connect(print_renames, &QPushButton::clicked, this, [&]()
        {
            if (!activeStory.has_value()) return;
            auto renames = activeStory.value().devGetRenames();
            QStringList test;
            auto i = 0;
            for (auto& entry : renames)
            {
                ++i;
                QString result = QString::number(i) + "\nKey: " + entry.key + "\nRel Path: " + entry.relPath;
                (entry.origRelPath.has_value())
                    ? result = result + "\nOrig Path: " + entry.origRelPath.value()
                    : result = result + "\nNew: " + QString((entry.typeIfNewOrCut.value() == Path::Type::Dir) ? "dir" : "file");
                test << result;
            }
            Io::writeFile((Ud::userData(Ud::Op::GetDocs) / "__Renames.txt"), test.join("\n\n"));
        });
}

void Fernanda::setName()
{
    QString name;
    (isDev)
        ? name = "fernanda-dev"
        : name = "fernanda";
    setWindowTitle(name);
    Ud::setName(name);
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
    connect(this, &Fernanda::sendItems, pane, &Pane::receiveItems);
    connect(this, &Fernanda::sendEditsList, pane, &Pane::receiveEditsList);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
    connect(pane, &Pane::askDomMove, this, &Fernanda::domMove);
    connect(pane, &Pane::askAddElement, this, &Fernanda::domAdd);
    connect(pane, &Pane::askRenameElement, this, &Fernanda::domRename);
    connect(pane, &Pane::askCutElement, this, &Fernanda::domCut);
    connect(pane, &Pane::askHasProject, this, &Fernanda::replyHasProject);
    connect(pane, &Pane::askSendToEditor, this, &Fernanda::handleEditorText);
    connect(textEditor, &TextEditor::askFontSliderZoom, this, &Fernanda::handleEditorZoom);
    connect(textEditor, &TextEditor::askHasProject, this, &Fernanda::replyHasProject);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::sendEditedText);
    connect(this, &Fernanda::startAutoTempSave, this, [&]() { autoTempSave->start(30000); });
    connect(autoTempSave, &QTimer::timeout, this, [&]() { activeStory.value().autoTempSave(textEditor->toPlainText()); });
    connect(pane, &Pane::askSetExpansion, this, [&](QString key, bool isExpanded) { activeStory.value().setItemExpansion(key, isExpanded); });
    connect(textEditor, &TextEditor::askNavNext, pane, [&]() { pane->nav(Pane::Nav::Next); });
    connect(textEditor, &TextEditor::askNavPrevious, pane, [&]() { pane->nav(Pane::Nav::Previous); });
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
    makeViewMenu();
    makeHelpMenu();
}

void Fernanda::makeFileMenu()
{
    auto* file = menuBar->addMenu(tr("&File"));
    auto* new_story = new QAction(tr("&New project..."), this);
    auto* open_story = new QAction(tr("&Open an existing project..."), this);
    auto* save = new QAction(tr("&Save"), this);
    auto* quit = new QAction(tr("&Quit"), this);
    save->setShortcut(Qt::CTRL | Qt::Key_S);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(new_story, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new story..."), Ud::userData(Ud::Op::GetDocs), tr("Fernanda story file (*.story)"));
            openStory(file_name);
        });
    connect(open_story, &QAction::triggered, this, [&]()
        {
            auto file_name = QFileDialog::getOpenFileName(this, tr("Open an existing story..."), Ud::userData(Ud::Op::GetDocs), tr("Fernanda story file (*.story)"));
            openStory(file_name);
        });
    connect(save, &QAction::triggered, this, &Fernanda::fileSave);
    connect(quit, &QAction::triggered, this, &QCoreApplication::quit, Qt::QueuedConnection);
    file->addAction(new_story);
    file->addAction(open_story);
    file->addSeparator();
    file->addAction(save);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeViewMenu() // Clean Me
{
    auto user_data = Ud::userData(Ud::Op::GetUserData);
    auto win_theme_list = Res::iterateResources(":\\themes\\window\\", "*.fernanda_wintheme", user_data, Res::Type::WindowTheme);
    windowThemes = makeViewToggles(win_theme_list, &Fernanda::setWindowStyle);
    loadViewConfig(windowThemes->actions(), "window", "wintheme", ":\\themes\\window\\Light.fernanda_wintheme");
    auto editor_theme_list = Res::iterateResources(":\\themes\\editor\\", "*.fernanda_theme", user_data, Res::Type::EditorTheme);
    editorThemes = makeViewToggles(editor_theme_list, &Fernanda::setEditorStyle);
    loadViewConfig(editorThemes->actions(), "editor", "theme", ":\\themes\\editor\\Amber.fernanda_theme");
    QVector<Res::DataPair> font_list;
    auto ttfs = Res::iterateResources(":\\fonts\\", "*.ttf", user_data, Res::Type::Font);
    font_list << Res::iterateResources(":\\fonts\\", "*.otf", user_data, Res::Type::Font, ttfs);
    editorFonts = makeViewToggles(font_list, &Fernanda::setEditorFont);
    loadViewConfig(editorFonts->actions(), "editor", "font", "Cascadia Mono");

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
    fontSlider->setMaximum(40);
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

    auto* toggle_shadow = new QAction(tr("&Toggle editor shadow"), this);
    connect(toggle_shadow, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasShadow, "editor", "shadow", checked, Toggle::Theme);
        });
    toggle_shadow->setCheckable(true);
    loadMenuToggle(toggle_shadow, "editor", "shadow", false);

    auto* toggle_keyfilter = new QAction(tr("&Toggle key filter"), this);
    connect(toggle_keyfilter, &QAction::toggled, this, [&](bool checked)
        {
            sendKeyfilterToggle(checked);
            Ud::saveConfig("editor", "key_filter", checked);
        });
    toggle_keyfilter->setCheckable(true);
    loadMenuToggle(toggle_keyfilter, "editor", "key_filter", true);

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

    auto* toggle_indicator = new QAction(tr("&Toggle indicator"), this);
    connect(toggle_indicator, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(indicator, "window", "indicator", checked);
        });
    toggle_indicator->setCheckable(true);
    loadMenuToggle(toggle_indicator, "window", "indicator", false);

    auto* toggle_line_pos = new QAction(tr("&Toggle line position"), this);
    connect(toggle_line_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.linePos, "window", "position_line", checked, Toggle::Pos);
        });
    toggle_line_pos->setCheckable(true);
    loadMenuToggle(toggle_line_pos, "window", "position_line", true);

    auto* toggle_col_pos = new QAction(tr("&Toggle column position"), this);
    connect(toggle_col_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.colPos, "window", "position_column", checked, Toggle::Pos);
        });
    toggle_col_pos->setCheckable(true);
    loadMenuToggle(toggle_col_pos, "window", "position_column", true);

    auto* toggle_line_count = new QAction(tr("&Toggle line count"), this);
    connect(toggle_line_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.lineCount, "window", "count_lines", checked, Toggle::Count);
        });
    toggle_line_count->setCheckable(true);
    loadMenuToggle(toggle_line_count, "window", "count_lines", true);

    auto* toggle_word_count = new QAction(tr("&Toggle word count"), this);
    connect(toggle_word_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.wordCount, "window", "count_words", checked, Toggle::Count);
        });
    toggle_word_count->setCheckable(true);
    loadMenuToggle(toggle_word_count, "window", "count_words", true);

    auto* toggle_char_count = new QAction(tr("&Toggle character count"), this);
    connect(toggle_char_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(indicator->has.charCount, "window", "count_characters", checked, Toggle::Count);
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
    view->addAction(toggle_shadow);
    view->addAction(toggle_keyfilter);
    view->addAction(toggle_line_numbers);
    view->addAction(toggle_scrolls);
    view->addSeparator();
    view->addAction(toggle_indicator);
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
    auto* shortcuts = new QAction(tr("&Shortcuts..."), this);
    auto* about = new QAction(tr("&About Fernanda..."), this);
    //connect(projects_dir, &QAction::triggered, this, &Fernanda::helpProjects);
    load_recent->setCheckable(true);
    connect(load_recent, &QAction::toggled, this, [&](bool checked)
        {
            Ud::saveConfig("data", "load_most_recent", checked);
        });
    loadMenuToggle(load_recent, "data", "load_most_recent", false);
    connect(sample_project, &QAction::triggered, this, &Fernanda::helpMakeSampleProject);
    connect(sample_themes, &QAction::triggered, this, &Fernanda::helpMakeSampleRes);
    connect(shortcuts, &QAction::triggered, this, &Fernanda::helpShortcuts);
    connect(about, &QAction::triggered, this, &Fernanda::helpAbout);
    //help->addAction(projects_dir);
    help->addAction(load_recent);
    help->addSeparator();
    help->addAction(sample_project);
    help->addAction(sample_themes);
    help->addSeparator();
    help->addAction(shortcuts);
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
    splitter->loadConfig(geometry());
    auto has_project = Ud::loadConfig("data", "load_most_recent", false, Ud::Type::Bool).toBool();
    if (has_project)
    {
        auto project = Ud::loadConfig("data", "project").toString();
        if (!QFile(project).exists() || project.isEmpty()) return;
        openStory(project);
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
        if (Path::sanitize(action->data().toString()) == resource)
        {
            action->setChecked(true);
            return;
        }
    for (auto action : actions)
        if (Path::sanitize(action->data().toString()) == fallback)
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

void Fernanda::openStory(QString fileName, Story::Op opt)
{
    if (fileName.isEmpty())
    {
        colorBar->red();
        return;
    }
    activeStory = Story(fileName, opt);
    auto& story = activeStory.value();
    sendItems(story.items());
    Ud::saveConfig("data", "project", Path::sanitize(fileName));
    colorBar->green();
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
        auto theme_path = selection->data().toString();
        auto style_sheet = Io::readFile(":\\themes\\window_base.qss");
        if (hasWinTheme && baseOnly != WinStyle::BaseOnly)
        {
            auto theme_sheet = Io::readFile(theme_path);
            style_sheet = style_sheet + "\n\n" + Res::createStyleSheetFromTheme(Io::readFile(":\\themes\\window.qss"), theme_sheet);
        }
        sendScrollsToggle(hasWinTheme);
        Ud::saveConfig("window", "wintheme", Path::sanitize(theme_path));
        return style_sheet;
    }
    return nullptr;
}

void Fernanda::setWindowStyle()
{
    setStyleSheet(windowStyle());
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = Io::readFile(":\\themes\\editor_base.qss");
        QString cursor_color = nullptr;
        QString under_cursor_color = nullptr;
        if (hasTheme)
        {
            auto theme_sheet = Io::readFile(theme_path);
            style_sheet = style_sheet + "\n\n" + Res::createStyleSheetFromTheme(Io::readFile(":\\themes\\editor.qss"), theme_sheet);
            QRegularExpressionMatch match_cursor = Uni::regex(Uni::Re::ThemeSheetCursor).match(theme_sheet);
            QRegularExpressionMatch match_under_cursor = Uni::regex(Uni::Re::ThemeSheetCursorUnder).match(theme_sheet);
            cursor_color = match_cursor.captured(2);
            under_cursor_color = match_under_cursor.captured(2);
        }
        if (hasShadow)
            style_sheet = style_sheet + "\n\n" + Io::readFile(":\\themes\\shadow.qss");
        overlay->setStyleSheet(style_sheet);
        underlay->setStyleSheet(style_sheet);
        textEditor->setStyleSheet(style_sheet);
        textEditor->cursorColorHex = cursor_color;
        textEditor->cursorUnderColorHex = under_cursor_color;
        Ud::saveConfig("editor", "theme", Path::sanitize(theme_path));
    }
}

void Fernanda::setEditorFont()
{
    if (auto selection = editorFonts->checkedAction(); selection != nullptr)
    {
        auto font = selection->data().toString();
        textEditor->setFont(font, fontSlider->value());
        Ud::saveConfig("editor", "font", font);
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
        aot->setText(Uni::ico(Uni::Ico::Balloon));
    }
    else
    {
        setWindowFlags(windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        aot->setText(Uni::ico(Uni::Ico::Pushpin));
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
        (alignment == "Bottom")
            ? colorBar->align(Qt::AlignBottom)
            : colorBar->align(Qt::AlignTop);
        Ud::saveConfig("window", "bar_alignment", alignment);
    }
}

void Fernanda::toggleGlobals(bool& globalBool, QString group, QString valueName, bool value, Toggle type)
{
    globalBool = value;
    Ud::saveConfig(group, valueName, value);
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

/*void Fernanda::helpProjects() // WIP
{
    //
}*/

void Fernanda::helpMakeSampleProject()
{
    // ask to save if another project is open
    auto path = Ud::userData(Ud::Op::GetDocs) / "Candide.story";
    openStory(path, Story::Op::Sample);
}

void Fernanda::helpMakeSampleRes()
{
    auto path = Ud::userData(Ud::Op::GetUserData);
    Sample::makeRc(path.toStdString());
    colorBar->pastels();
    QMessageBox alert;
    alert.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    alert.setWindowTitle("Hey!");
    alert.setText(Uni::samples());
    auto ok = alert.addButton(tr("Okay"), QMessageBox::AcceptRole);
    alert.setDefaultButton(ok);
    alert.exec();
}

void Fernanda::helpShortcuts()
{
    QMessageBox shortcuts;
    shortcuts.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    shortcuts.setWindowTitle("Shortcuts");
    shortcuts.setText(Uni::shortcuts());
    auto ok = shortcuts.addButton(tr("Okay"), QMessageBox::AcceptRole);
    shortcuts.setDefaultButton(ok);
    shortcuts.exec();
}

void Fernanda::helpAbout()
{
    QMessageBox about;
    about.setStyleSheet(windowStyle(WinStyle::BaseOnly));
    about.setWindowTitle("About");
    about.setText(Uni::about());
    auto ok = about.addButton(tr("Okay"), QMessageBox::AcceptRole);
    auto qt = about.addButton(tr("About Qt"), QMessageBox::AcceptRole);
    connect(qt, &QPushButton::clicked, this, QApplication::aboutQt);
    about.setDefaultButton(ok);
    about.exec();
}

void Fernanda::handleEditorText(QString key)
{
    if (overlay->isVisible())
        overlay->hide();
    auto& story = activeStory.value();
    if (!textEditor->handleKeySwap(story.key(), key)) return;
    auto text = story.tempSaveOld_openNew(key, textEditor->toPlainText());
    textEditor->handleTextSwap(key, text);
    startAutoTempSave();
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
    story.cut(key);
    sendItems(story.items());
}

void Fernanda::cycleCoreEditorThemes()
{

    auto actions = editorThemes->actions();
    auto current_theme = editorThemes->checkedAction();
    auto text = current_theme->text();
    auto break_it = false;
    for (auto& action : actions)
    {
        auto action_text = action->text();
        if (text != "Amber" && text != "Green" && action_text == "Amber")
            break_it = true;
        else if (text == "Amber" && action_text == "Green")
            break_it = true;
        else if (text == "Green" && action_text == "Grey")
            break_it = true;
        if (break_it)
        {
            action->setChecked(true);
            break;
        }
    }
}

// fernanda.cpp, fernanda