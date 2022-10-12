// fernanda.cpp, fernanda

#include "fernanda.h"

Fernanda::Fernanda(QWidget* parent)
    : QMainWindow(parent)
{
    handleQtUi();
    addWidgets();
    connections();
    UD::userData(UD::Op::Create, ferName);
    makeMenuBar();
    loadOpeningConfigs();
    //tests();
}

void Fernanda::tests()
{
    statusBar()->addPermanentWidget(test1, 0);
    test1->setStyleSheet("color:white;");
    for (auto& tuple : QVector<std::tuple<QPushButton*, QString>>{
        std::tuple<QPushButton*, QString>(test2, "background:red;color:white;"),
        std::tuple<QPushButton*, QString>(test3, "background:green;color:white;"),
        })
    {
        auto button = std::get<0>(tuple);
        auto& style = std::get<1>(tuple);
        button->setText("test");
        button->setStyleSheet(style);
        statusBar()->addPermanentWidget(button, 0);
    }
    connect(test2, &QPushButton::clicked, this, [&]() {});
    connect(test3, &QPushButton::clicked, this, [&]() {});
}

void Fernanda::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (isInitialized || event->spontaneous()) return;
    // popup if temps exist
    QTimer::singleShot(1000, this, [&]() { colorBarOn(Color::StartUp); });
    isInitialized = true;
}

void Fernanda::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    UD::saveConfig("window", "position", geometry());
}

void Fernanda::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    UD::saveConfig("window", "position", geometry());
}

void Fernanda::closeEvent(QCloseEvent* event)
{
    auto state = windowState();
    UD::saveConfig("window", "state", state.toInt());
    setWindowState(Qt::WindowState::WindowActive);
    if (currentProject.has_value() && currentProject.value().hasEdits())
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
            colorBarOn(Color::Green);
            return;
        }
    }
    UD::clearFiles(UD::userData(UD::Op::GetTemp));
    event->accept();
}

void Fernanda::handleQtUi()
{
    ui.setupUi(this);
    ui.mainToolBar->deleteLater();
    statusBar()->setSizeGripEnabled(true);
    setWindowTitle(ferName);
}

void Fernanda::addWidgets()
{
    // Create layout
    QWidget* full_layout = new QWidget(this);
    QStackedLayout* main_stack = new QStackedLayout(full_layout);
    QWidget* editor_layout = new QWidget(this);
    QStackedLayout* editor_stack = new QStackedLayout(editor_layout);
    main_stack->addWidget(colorBar);
    main_stack->addWidget(splitter);
    editor_stack->addWidget(overlay);
    editor_stack->addWidget(textEditor);
    editor_stack->addWidget(underlay);
    splitter->addWidget(pane);
    splitter->addWidget(editor_layout);
    // Adjust layouts
    main_stack->setStackingMode(QStackedLayout::StackAll);
    editor_stack->setStackingMode(QStackedLayout::StackAll);
    // Adjust widgets
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    colorBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    colorBar->setMaximumHeight(3);
    colorBar->setTextVisible(false);
    colorBar->setRange(0, 100);
    colorBar->hide();
    for (auto& widget_to_size : QVector<QWidget*>{ pane, overlay, textEditor, underlay })
        widget_to_size->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    splitter->setCollapsible(0, true);
    splitter->setCollapsible(1, false);
    splitter->setStretchFactor(1, 100);
    textEditor->setReadOnly(true);
    //overlay->hide();
    setCentralWidget(full_layout);
    aot->setCheckable(true);
    aot->setText("\U0001F4CC");
    statusBar()->addPermanentWidget(positions, 0);
    statusBar()->addPermanentWidget(counters, 0);
    statusBar()->addPermanentWidget(spacer, 1);
    statusBar()->addPermanentWidget(aot, 0);
    statusBar()->setMaximumHeight(22);
    // Name widgets
    splitter->setObjectName("splitter");
    pane->setObjectName("pane");
    overlay->setObjectName("overlay");
    textEditor->setObjectName("textEditor");
    underlay->setObjectName("underlay");
    colorBar->setObjectName("colorBar");
    fontSlider->setObjectName("fontSlider");
    positions->setObjectName("positions");
    counters->setObjectName("counters");
    spacer->setObjectName("spacer");
    aot->setObjectName("aot");
}

void Fernanda::connections()
{
    connect(barTimer, &QTimer::timeout, this, [&]()
        {
            colorBar->hide();
            colorBar->reset();
        });
    connect(splitter, &QSplitter::splitterMoved, this, [&]()
        {
            UD::saveConfig("window", "splitter", splitter->saveState());
        });
    connect(textEditor, &TextEditor::askFontSliderZoom, this, [&](bool zoomDirection)
        {
            if (zoomDirection)
                fontSlider->setValue(fontSlider->value() + 2);
            else
                fontSlider->setValue(fontSlider->value() - 2);
        });
    connect(pane, &Pane::askHasProject, this, &Fernanda::replyHasProject);
    connect(textEditor, &TextEditor::askHasProject, this, &Fernanda::replyHasProject);
    connect(this, &Fernanda::sendLineHighlightToggle, textEditor, &TextEditor::toggleLineHighlight);
    connect(this, &Fernanda::sendLineNumberAreaToggle, textEditor, &TextEditor::toggleLineNumberArea);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
    connect(textEditor, &TextEditor::cursorPositionChanged, this, &Fernanda::updatePositions);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::updateCounters);
    connect(this, &Fernanda::sendItems, pane, &Pane::receiveItems);
    connect(pane, &Pane::askSendToEditor, this, &Fernanda::handleEditorText);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::sendEditedText);
    connect(this, &Fernanda::sendEditsList, pane, &Pane::receiveEditsList);
    connect(this, &Fernanda::saveCursors, textEditor, &TextEditor::storeCursors);
    connect(this, &Fernanda::applyCursors, textEditor, &TextEditor::recallCursors);
    connect(this, &Fernanda::saveUndoStacks, textEditor, &TextEditor::storeUndoStacks);
    connect(this, &Fernanda::applyUndoStacks, textEditor, &TextEditor::recallUndoStacks);
    connect(textEditor, &TextEditor::askNavPrevious, pane, &Pane::replyNavPrevious);
    connect(textEditor, &TextEditor::askNavNext, pane, &Pane::replyNavNext);
}

void Fernanda::makeMenuBar()
{
    makeFileMenu();
    makeViewMenu();
    makeHelpMenu();
}

void Fernanda::makeFileMenu()
{
    auto* file = menuBar()->addMenu(tr("&File"));
    auto* new_project = new QAction(tr("&New project..."), this);
    auto* open_project = new QAction(tr("&Open an existing project..."), this);
    auto* save = new QAction(tr("&Save"), this);
    auto* save_all = new QAction(tr("&Save all"), this);
    auto* quit = new QAction(tr("&Quit"), this);
    save->setShortcut(Qt::CTRL | Qt::Key_S);
    save_all->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(new_project, &QAction::triggered, this, [&]()
        {
            //auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new project"), UD::userData(UD::Op::GetDocs), tr("Fernanda project file (*.story)"));
            auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new project"), UD::userData(UD::Op::GetDocs), tr("Fernanda project folder (*.story)"));
            openProject(file_name);
        });
    connect(open_project, &QAction::triggered, this, [&]()
        {
            //auto file_name = QFileDialog::getOpenFileName(this, tr("Open an existing project"), UD::userData(UD::Op::GetDocs), tr("Fernanda project file (*.story)"));
            auto file_name = QFileDialog::getExistingDirectory(this, tr("Open an existing project"), UD::userData(UD::Op::GetDocs), QFileDialog::ShowDirsOnly);
            openProject(file_name);
        });
    connect(save, &QAction::triggered, this, &Fernanda::fileSave);
    connect(save_all, &QAction::triggered, this, &Fernanda::fileSaveAll);
    connect(quit, &QAction::triggered, this, &QCoreApplication::quit, Qt::QueuedConnection);
    file->addAction(new_project);
    file->addAction(open_project);
    file->addSeparator();
    file->addAction(save);
    file->addAction(save_all);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeViewMenu() // clean me
{
    auto user_data = UD::userData(UD::Op::GetUserData);
    auto win_theme_list = RC::iterateResources(":/themes/window/", "*.fernanda_wintheme", user_data, RC::ResType::WindowTheme);
    windowThemes = makeViewToggles(win_theme_list, &Fernanda::setWindowStyle);
    loadViewConfig(windowThemes->actions(), "window", "wintheme", ":/themes/window/light.fernanda_wintheme");
    auto editor_theme_list = RC::iterateResources(":/themes/editor/", "*.fernanda_theme", user_data, RC::ResType::EditorTheme);
    editorThemes = makeViewToggles(editor_theme_list, &Fernanda::setEditorStyle);
    loadViewConfig(editorThemes->actions(), "editor", "theme", ":/themes/editor/amber.fernanda_theme");
    auto font_list = RC::iterateResources(":/fonts/", "*.ttf", user_data, RC::ResType::Font);
    editorFonts = makeViewToggles(font_list, &Fernanda::setEditorFont);
    loadViewConfig(editorFonts->actions(), "editor", "font", "Cascadia Code");

    auto* toggle_win_theme = new QAction(tr("&Toggle window theme"), this);
    connect(toggle_win_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasWinTheme, "window", "wintheme_on", checked, Toggle::WinTheme);
        });
    toggle_win_theme->setCheckable(true);
    loadViewConfigToggle(toggle_win_theme, "window", "wintheme_on", true);

    auto* toggle_theme = new QAction(tr("&Toggle editor theme"), this);
    connect(toggle_theme, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(hasTheme, "editor", "theme_on", checked, Toggle::Theme);
        });
    toggle_theme->setCheckable(true);
    loadViewConfigToggle(toggle_theme, "editor", "theme_on", true);

    auto* font_size_label = new QAction(tr("&Set editor font size:"), this);
    font_size_label->setEnabled(false);
    auto* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(64);
    connect(fontSlider, &QSlider::valueChanged, this, [&](int value)
        {
            setEditorFont();
            UD::saveConfig("editor", "font_size", value);
        });

    QVector<std::tuple<QString, QString>> tab_list = {
        std::tuple<QString, QString>("20", "20 px"),
        std::tuple<QString, QString>("40", "40 px"),
        std::tuple<QString, QString>("60", "60 px"),
        std::tuple<QString, QString>("80", "80 px"),
    };
    tabStops = makeViewToggles(tab_list, &Fernanda::setTabStop);
    loadViewConfig(tabStops->actions(), "editor", "tab", "40");
    QVector<std::tuple<QString, QString>> wrap_list = {
        std::tuple<QString, QString>("NoWrap", "No wrap"),
        std::tuple<QString, QString>("WordWrap", "Wrap at word boundaries"),
        std::tuple<QString, QString>("WrapAnywhere", "Wrap anywhere"),
        std::tuple<QString, QString>("WrapAt", "Wrap at word boundaries or anywhere")
    };
    wrapModes = makeViewToggles(wrap_list, &Fernanda::setWrapMode);
    loadViewConfig(wrapModes->actions(), "editor", "wrap", "WrapAt");

    auto* toggle_aot = new QAction(tr("&Toggle always-on-top button"), this);
    connect(toggle_aot, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(aot, "window", "aot_button", checked);
        });
    toggle_aot->setCheckable(true);
    loadViewConfigToggle(toggle_aot, "window", "aot_button", false);

    auto* toggle_pane = new QAction(tr("&Toggle file pane"), this);
    connect(toggle_pane, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(pane, "window", "pane", checked);
        });
    toggle_pane->setCheckable(true);
    loadViewConfigToggle(toggle_pane, "window", "pane", true);

    auto* toggle_statusbar = new QAction(tr("&Toggle statusbar"), this);
    connect(toggle_statusbar, &QAction::toggled, this, [&](bool checked)
        {
            toggleWidget(statusBar(), "window", "statusbar", checked);
        });
    toggle_statusbar->setCheckable(true);
    loadViewConfigToggle(toggle_statusbar, "window", "statusbar", true);

    auto* toggle_line_highlight = new QAction(tr("&Toggle current line highlight"), this);
    connect(toggle_line_highlight, &QAction::toggled, this, [&](bool checked)
        {
            sendLineHighlightToggle(checked);
            UD::saveConfig("editor", "line_highlight", checked);
        });
    toggle_line_highlight->setCheckable(true);
    loadViewConfigToggle(toggle_line_highlight, "editor", "line_highlight", true);

    auto* toggle_line_numbers = new QAction(tr("&Toggle line number area"), this);
    connect(toggle_line_numbers, &QAction::toggled, this, [&](bool checked)
        {
            sendLineNumberAreaToggle(checked);
            UD::saveConfig("editor", "line_number_area", checked);
        });
    toggle_line_numbers->setCheckable(true);
    loadViewConfigToggle(toggle_line_numbers, "editor", "line_number_area", false);

    auto* toggle_line_pos = new QAction(tr("&Toggle line position"), this);
    connect(toggle_line_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(linePos, "window", "position_line", checked, Toggle::Pos);
        });
    toggle_line_pos->setCheckable(true);
    loadViewConfigToggle(toggle_line_pos, "window", "position_line", false);

    auto* toggle_col_pos = new QAction(tr("&Toggle column position"), this);
    connect(toggle_col_pos, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(colPos, "window", "position_column", checked, Toggle::Pos);
        });
    toggle_col_pos->setCheckable(true);
    loadViewConfigToggle(toggle_col_pos, "window", "position_column", false);

    auto* toggle_line_count = new QAction(tr("&Toggle line count"), this);
    connect(toggle_line_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(lineCount, "window", "count_lines", checked, Toggle::Count);
        });
    toggle_line_count->setCheckable(true);
    loadViewConfigToggle(toggle_line_count, "window", "count_lines", false);

    auto* toggle_word_count = new QAction(tr("&Toggle word count"), this);
    connect(toggle_word_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(wordCount, "window", "count_words", checked, Toggle::Count);
        });
    toggle_word_count->setCheckable(true);
    loadViewConfigToggle(toggle_word_count, "window", "count_words", false);

    auto* toggle_char_count = new QAction(tr("&Toggle character count"), this);
    connect(toggle_char_count, &QAction::toggled, this, [&](bool checked)
        {
            toggleGlobals(charCount, "window", "count_characters", checked, Toggle::Count);
        });
    toggle_char_count->setCheckable(true);
    loadViewConfigToggle(toggle_char_count, "window", "count_characters", false);

    auto* view = menuBar()->addMenu(tr("&View"));
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
    view->addSeparator();
    view->addAction(toggle_aot);
    view->addAction(toggle_pane);
    view->addAction(toggle_statusbar);
    view->addSeparator();
    view->addAction(toggle_line_highlight);
    view->addAction(toggle_line_numbers);
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
    auto* help = menuBar()->addMenu(tr("&Help"));
    //auto* projects_dir = new QAction(tr("&Set default projects directory..."), this);
    auto* sample_themes = new QAction(tr("&Generate sample themes"), this);
    auto* sample_project = new QAction(tr("&Generate sample project"), this);
    auto* about = new QAction(tr("&About Fernanda"), this);
    //connect(projects_dir, &QAction::triggered, this, &Fernanda::helpProjects);
    connect(sample_themes, &QAction::triggered, this, &Fernanda::helpMakeSampleRes);
    connect(sample_project, &QAction::triggered, this, &Fernanda::helpMakeSampleProject);
    connect(about, &QAction::triggered, this, &Fernanda::helpAbout);
    //help->addAction(projects_dir);
    //help->addSeparator();
    help->addAction(sample_themes);
    help->addAction(sample_project);
    help->addSeparator();
    help->addAction(about);
}

QActionGroup* Fernanda::makeViewToggles(QVector<std::tuple<QString, QString>>& itemAndLabelPairs, void (Fernanda::* slot)())
{
    auto* group = new QActionGroup(this);
    for (auto& itemAndLabelPair : itemAndLabelPairs)
    {
        auto& data = std::get<0>(itemAndLabelPair);
        auto label = std::get<1>(itemAndLabelPair).toUtf8();
        auto* action = new QAction(tr(label), this);
        action->setData(data);
        connect(action, &QAction::toggled, this, slot);
        action->setCheckable(true);
        group->addAction(action);
    }
    group->setExclusive(true);
    return group;
}

void Fernanda::colorBarOn(Color style)
{
    if (style == Color::None) return;
    colorBarStyle(style);
    auto* bar_fill = new QTimeLine(125, this);
    connect(bar_fill, &QTimeLine::frameChanged, colorBar, &QProgressBar::setValue);
    bar_fill->setFrameRange(0, 100);
    colorBar->show();
    barTimer->start(1000);
    bar_fill->start();
}

void Fernanda::colorBarStyle(Color style)
{
    QString style_sheet;
    switch (style) {
        case Color::Red: style_sheet = IO::readFile(":/themes/bar/red.qss"); break;
        case Color::Green: style_sheet = IO::readFile(":/themes/bar/green.qss"); break;
        case Color::StartUp: style_sheet = IO::readFile(":/themes/bar/start_up.qss"); break;
    }
    colorBar->setStyleSheet(style_sheet);
}

void Fernanda::loadOpeningConfigs()
{
    loadWindowConfigs();
    auto font_slider_position = UD::loadConfig("editor", "font_size", 14, QMetaType::Int);
    fontSlider->setValue(font_slider_position.toInt());
    auto splitter_pos = UD::loadConfig("window", "splitter");
    splitter->restoreState(splitter_pos.toByteArray());
}

void Fernanda::loadWindowConfigs()
{
    auto win_pos = UD::loadConfig("window", "position", QRect(0, 0, 1000, 666), QMetaType::QRect);
    setGeometry(win_pos.toRect());
    auto win_state = UD::loadConfig("window", "state").toInt();
    if (win_state == 1) setWindowState(Qt::WindowState::WindowMinimized);
    else if (win_state == 2) setWindowState(Qt::WindowState::WindowMaximized);
    else if (win_state == 4) setWindowState(Qt::WindowState::WindowFullScreen);
    auto aot_state = UD::loadConfig("window", "always_on_top", false);
    aot->setChecked(aot_state.toBool());
}

void Fernanda::loadViewConfig(QVector<QAction*> actions, QString group, QString valueName, QVariant fallback)
{
    auto resource = UD::loadConfig(group, valueName, fallback);
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

void Fernanda::loadViewConfigToggle(QAction* action, QString group, QString valueName, QVariant fallback)
{
    auto toggle_state = UD::loadConfig(group, valueName, fallback, QMetaType::Bool).toBool();
    action->setChecked(!toggle_state); // whyyyyyyyyy
    action->setChecked(toggle_state);
}

void Fernanda::openProject(QString fileName)
{
    if (fileName.isEmpty())
    {
        colorBarOn(Color::Red);
        return;
    }
    currentProject = Project(fileName);
    sendItems(currentProject.value().items());
    UD::saveConfig("data", "project", fileName);
    colorBarOn(Color::Green);
}

void Fernanda::setWindowStyle()
{
    if (auto selection = windowThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = RC::createStyleSheetFromTheme(IO::readFile(":/themes/window.qss"), IO::readFile(theme_path));
        if (!hasWinTheme)
            setStyleSheet("");
        else
            setStyleSheet(style_sheet);
        UD::saveConfig("window", "wintheme", theme_path);
    }
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = RC::createStyleSheetFromTheme(IO::readFile(":/themes/editor.qss"), IO::readFile(theme_path));
        auto theme_cursor = IO::readFile(theme_path);
        QRegularExpressionMatch match_cursor = QRegularExpression("(@cursorColor; = )(.*)(;$)").match(theme_cursor);
        QString cursor_color = match_cursor.captured(2);
        if (!hasTheme)
        {
            overlay->setStyleSheet("");
            textEditor->setStyleSheet("");
            underlay->setStyleSheet("");
            textEditor->cursorColorHex = "";
        }
        else
        {
            overlay->setStyleSheet(style_sheet);
            textEditor->setStyleSheet(style_sheet);
            underlay->setStyleSheet(style_sheet);
            textEditor->cursorColorHex = cursor_color;
        }
        UD::saveConfig("editor", "theme", theme_path);
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
        UD::saveConfig("editor", "font", selection->data().toString());
    }
}

void Fernanda::toggleWidget(QWidget* widget, QString group, QString valueName, bool value)
{
    widget->setVisible(value);
    UD::saveConfig(group, valueName, value);
}

void Fernanda::aotToggled(bool checked)
{
    if (checked)
    {
        setWindowFlags(windowFlags() | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        aot->setText("\U0001F388");
    }
    else
    {
        setWindowFlags(windowFlags() ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        aot->setText("\U0001F4CC");
    }
    show();
    UD::saveConfig("window", "always_on_top", checked);
}

void Fernanda::setTabStop()
{
    if (auto selection = tabStops->checkedAction(); selection != nullptr)
    {
        auto distance = selection->data().toInt();
        textEditor->setTabStopDistance(distance);
        UD::saveConfig("editor", "tab", distance);
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
        UD::saveConfig("editor", "wrap", mode);
    }
}

void Fernanda::toggleGlobals(bool& globalBool, QString group, QString valueName, bool value, Toggle type)
{
    globalBool = value;
    UD::saveConfig(group, valueName, value);
    switch (type) {
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
    if (linePos)
        elements << "ln " + QString::number(line_pos + 1);
    if (colPos)
        elements << "col " + QString::number(col_pos + 1);
    auto pos_display = elements.join(", ");
    positions->setText(pos_display);
}

void Fernanda::updateCounters()
{
    const auto text = textEditor->toPlainText();
    const auto line_count = textEditor->blockCount();
    const auto word_count = text.split(QRegularExpression("(\\s|\\n|\\r)+")).count();
    const auto char_count = text.count();
    QStringList elements;
    if (lineCount)
        elements << QString::number(line_count) + " lines";
    if (wordCount)
        elements << QString::number(word_count - 1) + " words"; // - 1, whyyyyy
    // The above always displays at least 1, even when editor is blank?
    if (charCount)
        elements << QString::number(char_count) + " chars";
    auto counter_display = elements.join(", ");
    counters->setText(counter_display);
}

void Fernanda::fileSave()
{
    if (!currentProject.has_value()) return;
    auto& project = currentProject.value();
    project.saveCurrent(textEditor->toPlainText());
    textEditor->textChanged();
    colorBarOn(Color::Green);
}

void Fernanda::fileSaveAll()
{
    fileSave();
    currentProject.value().saveNonActiveFiles();
    textEditor->textChanged();
    colorBarOn(Color::StartUp);
}

/*void Fernanda::helpProjects()
{
    //
}*/

void Fernanda::helpMakeSampleRes()
{
    auto path = UD::userData(UD::Op::GetUserData);
    Sample::makeRc(path.toStdString());
    // popup to tell user to restart to see changes
}

void Fernanda::helpMakeSampleProject()
{
    // ask to save if another project is open
    auto path = UD::userData(UD::Op::GetDocs) + "/Candide.story";
    currentProject = Project(path, Project::SP::MakeSample);
    sendItems(currentProject.value().items());
    UD::saveConfig("data", "project", path);
    colorBarOn(Color::Green);
}

void Fernanda::helpAbout()
{
    QMessageBox about;
    about.setWindowTitle("About");
    about.setText("Fernanda is a personal project and a work-in-progress.");
    QAbstractButton* ok = about.addButton(tr("Okay"), QMessageBox::AcceptRole);
    about.exec();
}

void Fernanda::handleEditorText(QString relFilePath)
{
    if (!currentProject.has_value()) return;
    if (overlay->isVisible())
        overlay->hide();
    if (textEditor->isReadOnly())
        textEditor->setReadOnly(false);
    auto& project = currentProject.value();
    auto old_file = project.getCurrentRelPath();
    if (old_file == relFilePath) // can we do this in project?
    {
        textEditor->setFocus();
        return;
    }
    if (old_file != nullptr) // can we do this in project?
    {
        saveCursors(old_file);
        saveUndoStacks(old_file);
    }
    auto text = project.handleIO(relFilePath, textEditor->toPlainText());
    textEditor->clear();
    textEditor->setPlainText(text);
    applyCursors(relFilePath);
    applyUndoStacks(relFilePath);
    textEditor->setFocus();
}

void Fernanda::sendEditedText()
{
    if (!currentProject.has_value()) return;
    auto list = currentProject.value().handleEditedList(textEditor->toPlainText());
    sendEditsList(list);
}

bool Fernanda::replyHasProject()
{
    if (currentProject.has_value()) return true;
    else return false;
}

// TODO:
// auto save
// save redo stack
// method to swap rel paths for these lists to persist across file moves
// highlighted words count
// add QValidator in pane rename to prevent "/"
// version.h
// moveable docs folder
// move some of the widget-specific things in layout objects above, into their constructors?
// stick with "ask" / "reply" terminology for those types of signal/slot combos. Go back and change outliers.
// redo include check (from bottom to top, double-checking includes)
// check what overloaded/re-implemented functions are protected (and should be listed as protected in header)
// double check handleEditorText and see what is moveable to individual classes
// in delegate, add isDir and isFile functions that take the index as argument and read the UserRole data for "dir" or "file"
// then, in nav pane, check isDir or isFile, and if isDir && state.open/closed expand or not based on that (and implement in delegate also)

// fernanda.cpp, fernanda