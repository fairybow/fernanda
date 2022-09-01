#include "fernanda.h"

Fernanda::Fernanda(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    nameObjects();
    layoutObjects();
    makeConnections();
    createUserData(userDataName);
    loadMiscConfigsOnOpen();
    makeMenuBar();
    auto has_full_projects_data = loadProjectDataOnOpen();
    if (!has_full_projects_data)
        QTimer::singleShot(1250, this, &Fernanda::chooseProjectDataOnOpen);
    
    // Bug: After start-up, the first folder clicked in Pane is slow to load. After that, it's fine?
}

void Fernanda::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (wasInitialized || event->spontaneous()) return;
    textEditorTextChanged();
    QTimer::singleShot(1000, this, [&](){ startColorBar(ColorScheme::StartUp); });
    wasInitialized = true;
    // This does not actually halt the colorBar from loading until the window is visible. I think it works when the program starts minimized, but not when window is just obscured by, say, another program. This is probably normal behavior.
}

void Fernanda::nameObjects() 
{
    setWindowTitle(fernandaName);
    splitter->setObjectName("splitter");
    pane->setObjectName("pane");
    overlay->setObjectName("overlay");
    textEditor->setObjectName("textEditor");
    underlay->setObjectName("underlay");
    colorBar->setObjectName("colorBar");
    pathDisplay->setObjectName("pathDisplay");
    positions->setObjectName("positions");
    counters->setObjectName("counters");
    aot->setObjectName("aot");
    fontSlider->setObjectName("fontSlider");
}

void Fernanda::layoutObjects()
{
    ui.mainToolBar->deleteLater();

    QWidget* full_layout = new QWidget(this);
    QStackedLayout* main_stack = new QStackedLayout(full_layout);
    QWidget* editor_layout = new QWidget(this);
    QStackedLayout* editor_stack = new QStackedLayout(editor_layout);
    editor_stack->setStackingMode(QStackedLayout::StackAll);
    editor_stack->addWidget(overlay);
    editor_stack->addWidget(textEditor);
    editor_stack->addWidget(underlay);
    splitter->addWidget(pane);
    splitter->addWidget(editor_layout);
    splitter->setCollapsible(0, true);
    splitter->setCollapsible(1, false);
    splitter->setStretchFactor(1, 100);
    main_stack->setStackingMode(QStackedLayout::StackAll);
    main_stack->addWidget(colorBar);
    main_stack->addWidget(splitter);
    setCentralWidget(full_layout);

    colorBar->setMaximumHeight(3);
    colorBar->setTextVisible(false);
    colorBar->setRange(0, 100);
    colorBar->hide();

    colorBar->setAttribute(Qt::WA_TransparentForMouseEvents);
    overlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    statusBar()->addPermanentWidget(pathDisplay, 1);
    statusBar()->addPermanentWidget(positions, 0);
    statusBar()->addPermanentWidget(counters, 0);
    statusBar()->addPermanentWidget(aot, 0);
    statusBar()->setMaximumHeight(22);

    aot->setCheckable(true);
    aot->setText("\U0001F4CC");
    pane->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    overlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    textEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    underlay->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void Fernanda::makeConnections()
{
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, [&]() { autoSaveTimer->start(30000); });
    connect(autoSaveTimer, &QTimer::timeout, this, [&]()
        {
            auto& path = std::get<0>(pane->currentFile);
            ifPreviousFileIsFile(path);
        });
    connect(splitter, &QSplitter::splitterMoved, this, [&]()
        {
            saveConfig<QByteArray>("window", "splitter", splitter->saveState());
            // Splitter size is saving, but not being recalled by loadConfig
        });
    connect(barTimer, &QTimer::timeout, this, [&]()
        {
            colorBar->hide();
            colorBar->reset();
        });
    connect(textEditor, &TextEditor::askFontSliderZoom, this, [&](bool zoomDirection)
        {
            if (zoomDirection)
                fontSlider->setValue(fontSlider->value() + 2);
            else
                fontSlider->setValue(fontSlider->value() - 2);
        });
    connect(pane, &Pane::previousFileIsFile, this, &Fernanda::ifPreviousFileIsFile);
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, &Fernanda::ifPathDoesNotEqualPrevPath);
    connect(textEditor, &TextEditor::askNavPrevious, pane, &Pane::navPrevious);
    connect(textEditor, &TextEditor::askNavNext, pane, &Pane::navNext);
    connect(this, &Fernanda::sendLineHighlightToggle, textEditor, &TextEditor::toggleLineHighlight);
    connect(this, &Fernanda::sendLineNumberAreaToggle, textEditor, &TextEditor::toggleLineNumberArea);
    connect(pane, &Pane::changePathDisplay, this, &Fernanda::displayPath);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::textEditorTextChanged);
    connect(textEditor, &TextEditor::cursorPositionChanged, this, &Fernanda::updatePositions);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::updateCounters);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
}

void Fernanda::checkTempsOnOpen()
{
    if (!QDir(activeTemp).isEmpty())
    {
        alert(ColorScheme::None, "Fernanda may not have properly quit last time, and unsaved changes are available. Would you like to recover them?", "No", &Fernanda::clearTempFiles, "Yes", "Unsaved changes");
        /*if (!QDir(activeTemp).isEmpty())
        {
            // Any way to mark them dirty on load? Pane delegate does not paint the entries "dirty" until they're clicked on. Since files are marked dirty by comparing to a stored cleanText string, I don't know if it's possible to mark them dirty beforehand in any simple way.
        }*/
    }
}

void Fernanda::closeEvent(QCloseEvent* event)
{
    auto state = windowState() & Qt::WindowState::WindowMaximized;
    saveConfig<bool>("window", "max", state);
    setWindowState(Qt::WindowState::WindowActive);

    if (!pane->paneDelegate->dirtyIndexes.isEmpty() || !dirtyFiles.isEmpty())
    {
        auto close = QMessageBox::question(this, fernandaName, "You may have unsaved changes. Are you sure you want to quit?", QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
        if (close == QMessageBox::No)
        {
            if (state == Qt::WindowState::WindowMaximized)
                setWindowState(Qt::WindowState::WindowMaximized); // I would actually prefer to save *any* window state when closeEvent is triggered (before moving to active) and save to ini, then return to that state if close is canceled.
            event->ignore();
            return;
        }
    }
    clearTempFiles();
    event->accept();
}

void Fernanda::displayPath()
{
    auto& full_index_path = std::get<1>(pane->selectedIndex);
    std::filesystem::path index_path = full_index_path.toStdString();
    auto index_root = index_path.root_name();
    auto index_drive_letter = QString::fromStdString(index_root.string()) + "\\>";
    auto index_name = index_path.filename();
    auto _index_name = QString::fromStdString(index_name.string());

    auto& full_file_path = std::get<0>(pane->currentFile);
    std::filesystem::path file_path = full_file_path.toStdString();
    auto file_root = file_path.root_name();
    auto file_drive_letter = QString::fromStdString(file_root.string()) + "\\>";
    auto file_name = file_path.filename();
    auto _file_name = QString::fromStdString(file_name.string());
    
    QString italics_1 = "";
    QString italics_2 = "";
    if (dirtyFiles.contains(full_file_path))
    {
        italics_1 = "<i>";
        italics_2 = "*</i>";
    }
    QString par_1 = "";
    QString par_2 = "";
    if (!full_file_path.isEmpty())
    {
        par_1 = " (";
        par_2 = ")";
    }

    if (index_path == file_path)
        pathDisplay->setText(file_drive_letter + italics_1 + _file_name + italics_2);
    else
        pathDisplay->setText(index_drive_letter + _index_name + par_1 + italics_1 + _file_name + italics_2 + par_2);
}

void Fernanda::makeMenuBar()
{
    makeFileMenu();
    makeViewMenu();
    makeHelpMenu();
}

void Fernanda::makeFileMenu()
{
    auto* open_project = new QAction(tr("&Open a project..."), this);
    open_project->setStatusTip(tr("Open a project"));
    connect(open_project, &QAction::triggered, this, &Fernanda::fileMenuOpenProject);

    auto* new_project = new QAction(tr("&Create a new project folder...(WIP)"), this);
    new_project->setStatusTip(tr("Create a new project folder"));
    connect(new_project, &QAction::triggered, this, &Fernanda::fileMenuNewProject);

    auto* new_file = new QAction(tr("&Create a new file..."), this);
    new_file->setShortcut(Qt::CTRL | Qt::Key_N);
    new_file->setStatusTip(tr("Create a new file"));
    connect(new_file, &QAction::triggered, this, &Fernanda::fileMenuNew);

    auto* new_subfolder = new QAction(tr("&Create a new subfolder..."), this);
    new_subfolder->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    new_subfolder->setStatusTip(tr("Create a new folder"));
    connect(new_subfolder, &QAction::triggered, this, &Fernanda::fileMenuNewSubfolder);

    auto* save = new QAction(tr("&Save"), this);
    save->setShortcut(Qt::CTRL | Qt::Key_S);
    save->setStatusTip(tr("Save current document"));
    connect(save, &QAction::triggered, this, &Fernanda::fileMenuSave);

    auto* save_all = new QAction(tr("&Save all"), this);
    save_all->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    save_all->setStatusTip(tr("Save all documents in the current project"));
    connect(save_all, &QAction::triggered, this, &Fernanda::fileMenuSaveAll);

    auto* quit = new QAction(tr("&Quit"), this);
    quit->setShortcut(Qt::CTRL | Qt::Key_Q);
    quit->setStatusTip(tr("Quit"));
    connect(quit, &QAction::triggered, this, &QCoreApplication::quit, Qt::QueuedConnection);

    auto* file = menuBar()->addMenu(tr("&File"));
    file->addAction(open_project);
    file->addAction(new_project);
    file->addSeparator();
    file->addAction(new_file);
    file->addAction(new_subfolder);
    file->addSeparator();
    file->addAction(save);
    file->addAction(save_all);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeViewMenu()
{
    QList<std::tuple<QString, QString>> win_theme_list = iterateResources(":/themes/window/", "*.fernanda_wintheme", userData, ResourceType::WindowTheme);
    windowThemes = createMenuToggles(win_theme_list, &Fernanda::setWindowStyle);

    QList<std::tuple<QString, QString>> editor_theme_list = iterateResources(":/themes/editor/", "*.fernanda_theme", userData, ResourceType::EditorTheme);
    editorThemes = createMenuToggles(editor_theme_list, &Fernanda::setEditorStyle);

    QList<std::tuple<QString, QString>> font_list = iterateResources(":/fonts/", "*.ttf", userData, ResourceType::Font);
    editorFonts = createMenuToggles(font_list, &Fernanda::setEditorFont);

    auto* font_size_label = new QAction(tr("&Set editor font size:"), this);
    font_size_label->setStatusTip(tr("Set editor font size"));
    font_size_label->setEnabled(false);

    auto* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(64);

    QList<std::tuple<QString, QString>> tab_list = {
        std::tuple<QString, QString>("20", "20 px"),
        std::tuple<QString, QString>("40", "40 px"),
        std::tuple<QString, QString>("60", "60 px"),
        std::tuple<QString, QString>("80", "80 px"),
    };
    tabStops = createMenuToggles(tab_list, &Fernanda::setTabStop);

    QList<std::tuple<QString, QString>> wrap_list = {
        std::tuple<QString, QString>("NoWrap", "No wrap"),
        std::tuple<QString, QString>("WordWrap", "Wrap at word boundaries"),
        std::tuple<QString, QString>("WrapAnywhere", "Wrap anywhere"),
        std::tuple<QString, QString>("WrapAt", "Wrap at word boundaries or anywhere")
    };
    wrapModes = createMenuToggles(wrap_list, &Fernanda::setWrapMode);

    auto* toggle_aot = new QAction(tr("&Toggle always-on-top button"), this);
    toggle_aot->setStatusTip(tr("Toggle always-on-top button"));
    connect(toggle_aot, &QAction::toggled, this, [&](bool checked)
        {
            menuToggles(aot, "window", "aot_button_visible", checked);
        });
    toggle_aot->setCheckable(true);

    auto* toggle_pane = new QAction(tr("&Toggle file pane"), this);
    toggle_pane->setStatusTip(tr("Toggle file pane"));
    connect(toggle_pane, &QAction::toggled, this, [&](bool checked)
        {
            menuToggles(pane, "window", "pane_visible", checked);
        });
    toggle_pane->setCheckable(true);

    auto* toggle_extra_scrolls = new QAction(tr("&Toggle scroll previous and next buttons"), this);
    toggle_extra_scrolls->setStatusTip(tr("Toggle scroll previous and next buttons"));
    connect(toggle_extra_scrolls, &QAction::toggled, this, [&](bool checked)
        {
            menuToggles(textEditor->scrollPrevious, "editor", "scroll_previous_visible", checked);
            menuToggles(textEditor->scrollNext, "editor", "scroll_next_visible", checked);
        });
    toggle_extra_scrolls->setCheckable(true);

    auto* toggle_statusbar = new QAction(tr("&Toggle statusbar"), this);
    toggle_statusbar->setStatusTip(tr("Toggle statusbar"));
    connect(toggle_statusbar, &QAction::toggled, this, [&](bool checked)
        {
            menuToggles(statusBar(), "window", "statusbar_visible", checked);
        });
    toggle_statusbar->setCheckable(true);

    auto* toggle_line_highlight = new QAction(tr("&Toggle current line highlight"), this);
    toggle_line_highlight->setStatusTip(tr("Toggle current line highlight"));
    connect(toggle_line_highlight, &QAction::toggled, this, [&](bool checked)
        {
            sendLineHighlightToggle(checked);
            saveConfig<bool>("editor", "line_highlight", checked);
        });
    toggle_line_highlight->setCheckable(true);

    auto* toggle_key_filters = new QAction(tr("&Toggle key filters"), this);
    toggle_key_filters->setStatusTip(tr("Toggle key filters"));
    connect(toggle_key_filters, &QAction::toggled, this, [&](bool checked)
        {
            textEditor->hasKeyFilters = checked;
            saveConfig<bool>("editor", "key_filters", checked);
        });
    toggle_key_filters->setCheckable(true);

    auto* toggle_line_numbers = new QAction(tr("&Toggle line number area"), this);
    toggle_line_numbers->setStatusTip(tr("Toggle line number area"));
    connect(toggle_line_numbers, &QAction::toggled, this, [&](bool checked)
        {
            sendLineNumberAreaToggle(checked);
            saveConfig<bool>("editor", "line_number_area", checked);
        });
    toggle_line_numbers->setCheckable(true);

    auto* toggle_line_pos = new QAction(tr("&Toggle line position"), this);
    toggle_line_pos->setStatusTip(tr("Toggle line position"));
    connect(toggle_line_pos, &QAction::toggled, this, [&](bool checked)
        {
            togglePosAndCounts(linePos, "window", "show_line_position", checked);
        });
    toggle_line_pos->setCheckable(true);

    auto* toggle_col_pos = new QAction(tr("&Toggle column position"), this);
    toggle_col_pos->setStatusTip(tr("Toggle column position"));
    connect(toggle_col_pos, &QAction::toggled, this, [&](bool checked)
        {
            togglePosAndCounts(colPos, "window", "show_column_position", checked);
        });
    toggle_col_pos->setCheckable(true);

    auto* toggle_line_count = new QAction(tr("&Toggle line count"), this);
    toggle_line_count->setStatusTip(tr("Toggle line count"));
    connect(toggle_line_count, &QAction::toggled, this, [&](bool checked)
        {
            togglePosAndCounts(lineCount, "window", "show_line_count", checked);
        });
    toggle_line_count->setCheckable(true);

    auto* toggle_word_count = new QAction(tr("&Toggle word count"), this);
    toggle_word_count->setStatusTip(tr("Toggle word count"));
    connect(toggle_word_count, &QAction::toggled, this, [&](bool checked)
        {
            togglePosAndCounts(wordCount, "window", "show_word_count", checked);
        });
    toggle_word_count->setCheckable(true);

    auto* toggle_char_count = new QAction(tr("&Toggle character count"), this);
    toggle_char_count->setStatusTip(tr("Toggle character count"));
    connect(toggle_char_count, &QAction::toggled, this, [&](bool checked)
        {
            togglePosAndCounts(charCount, "window", "show_character_count", checked);
        });
    toggle_char_count->setCheckable(true);

    auto* view = menuBar()->addMenu(tr("&View"));
    auto* window_themes = view->addMenu(tr("&Set window theme"));
    window_themes->addActions(windowThemes->actions());
    auto* editor_themes = view->addMenu(tr("&Set editor theme"));
    editor_themes->addActions(editorThemes->actions());
    view->addSeparator();
    auto* fonts = view->addMenu(tr("&Set editor font"));
    fonts->addActions(editorFonts->actions());

    view->addAction(font_size_label);
    view->addAction(font_size);
    connect(fontSlider, &QSlider::valueChanged, this, [&](int value)
        {
            setEditorFont();
            saveConfig<int>("editor", "font_size", value);
        });
    view->addSeparator();
    auto* tab_stops = view->addMenu(tr("&Set editor tab stop distance"));
    tab_stops->addActions(tabStops->actions());

    auto* wrap_modes = view->addMenu(tr("&Set editor wrap mode"));
    wrap_modes->addActions(wrapModes->actions());
    view->addSeparator();
    view->addAction(toggle_aot);
    view->addAction(toggle_pane);
    view->addAction(toggle_extra_scrolls);
    view->addAction(toggle_statusbar);
    view->addSeparator();
    view->addAction(toggle_line_highlight);
    view->addAction(toggle_key_filters);
    view->addAction(toggle_line_numbers);
    view->addSeparator();
    auto* positions = view->addMenu(tr("&Show line and column positions"));
    positions->addAction(toggle_line_pos);
    positions->addAction(toggle_col_pos);
    auto* counters = view->addMenu(tr("&Show line, word, and character counts"));
    counters->addAction(toggle_line_count);
    counters->addAction(toggle_word_count);
    counters->addAction(toggle_char_count);

    loadResourceConfig(windowThemes->actions(), "window", "wintheme", ":/themes/window/light.fernanda_wintheme");
    loadResourceConfig(editorThemes->actions(), "editor", "theme", ":/themes/editor/amber.fernanda_theme");
    loadResourceConfig(editorFonts->actions(), "editor", "font", "Cascadia Code");
    loadResourceConfig(tabStops->actions(), "editor", "tab", "40");
    loadResourceConfig(wrapModes->actions(), "editor", "wrap", "WrapAt");
    loadMenuToggles(toggle_aot, "window", "aot_button_visible", false);
    loadMenuToggles(toggle_pane, "window", "pane_visible", true);
    loadMenuToggles(toggle_extra_scrolls, "editor", "scroll_previous_visible", true);
    loadMenuToggles(toggle_statusbar, "window", "statusbar_visible", true);
    loadMenuToggles(toggle_line_highlight, "editor", "line_highlight", true);
    loadMenuToggles(toggle_key_filters, "editor", "key_filters", true);
    loadMenuToggles(toggle_line_numbers, "editor", "line_number_area", true);

    loadMenuToggles(toggle_line_pos, "window", "show_line_position", false);
    loadMenuToggles(toggle_col_pos, "window", "show_column_position", false);
    loadMenuToggles(toggle_line_count, "window", "show_line_count", false);
    loadMenuToggles(toggle_word_count, "window", "show_word_count", false);
    loadMenuToggles(toggle_char_count, "window", "show_character_count", false);
}

void Fernanda::makeHelpMenu()
{
    auto* view_help = new QAction(tr("&View help"), this);
    view_help->setStatusTip(tr("View help"));
    //connect(view_help, &QAction::triggered, this, &Fernanda::);

    auto* set_projects_dir = new QAction(tr("&Change the default projects directory..."), this);
    set_projects_dir->setStatusTip(tr("Change the default projects directory"));
    connect(set_projects_dir, &QAction::triggered, this, &Fernanda::helpMenuSetProjectsDir);

    auto* sample = new QAction(tr("&Generate sample project"), this);
    sample->setStatusTip(tr("Generate sample project"));
    connect(sample, &QAction::triggered, this, &Fernanda::helpMenuMakeSample);

    auto* sample_themes = new QAction(tr("&Generate sample themes"), this);
    sample_themes->setStatusTip(tr("Generate sample themes"));
    connect(sample_themes, &QAction::triggered, this, &Fernanda::helpMenuMakeSampleUdRc);

    auto* about = new QAction(tr("&About Fernanda"), this);
    about->setStatusTip(tr("About Fernanda"));
    connect(about, &QAction::triggered, this, &Fernanda::helpMenuAbout);

    auto* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(view_help);
    help->addSeparator();
    help->addAction(set_projects_dir);
    help->addAction(sample);
    help->addAction(sample_themes);
    help->addSeparator();
    help->addAction(about);
}

QActionGroup* Fernanda::createMenuToggles(QList<std::tuple<QString, QString>>& itemAndLabel, void (Fernanda::* slot)())
{
    auto* group = new QActionGroup(this);
    for (auto& item : itemAndLabel)
    {
        auto& data = std::get<0>(item);
        auto label = std::get<1>(item).toUtf8();
        auto* action = new QAction(tr(label), this);
        action->setStatusTip(tr(label));
        action->setData(data);
        connect(action, &QAction::toggled, this, slot);
        action->setCheckable(true);
        group->addAction(action);
    }
    group->setExclusive(true);
    return group;
}

void Fernanda::createUserData(QString dataFolderName)
{
    userData = std::filesystem::path(QDir::homePath().toStdString()) / dataFolderName.toStdString();
    auto config_path = userData / configName.toStdString();
    config = QString::fromStdString(config_path.string());
    activeTemp = userData / std::string(".active_temp");
    backup = userData / std::string("backup");
    rollback = backup / std::string(".rollback");
    QList<std::filesystem::path> data_folders = { userData, activeTemp, backup, rollback };
    for (auto& folder : data_folders)
        if (!QDir(folder).exists())
            std::filesystem::create_directory(folder);
}

void Fernanda::clearTempFiles()
{
    for (auto& item : std::filesystem::directory_iterator(activeTemp))
        std::filesystem::remove_all(item);
}

const QString Fernanda::pathMaker(QString path, PathType type)
{
    std::filesystem::path data_folder;
    QString prefix;
    QString ext;

    if (type == PathType::Backup)
    {
        //
    }
    else if (type == PathType::Original)
        return path;
    else if (type == PathType::Rollback)
    {
        data_folder = rollback;
        prefix = "";
        ext = ".fernanda_bak";
    }
    else if (type == PathType::Temp)
    {
        data_folder = activeTemp;
        prefix = "~";
        ext = ".fernanda_temp";
    }

    auto proj_relative = std::filesystem::relative(path.toStdString(), defaultProjectsFolder.toStdString());
    std::filesystem::path proj_relative_parent = prefix.toStdString() + proj_relative.parent_path().string();
    std::filesystem::path file_name = prefix.toStdString() + proj_relative.filename().replace_extension(ext.toStdString()).string();
    auto derived_path = data_folder / proj_relative_parent / file_name;
    derived_path.make_preferred();

    return QString::fromStdString(derived_path.string());
}

void Fernanda::swap(QString path)
{
    std::filesystem::path rollback_path = pathMaker(path, PathType::Rollback).toStdString();
    std::filesystem::path temp_path = pathMaker(path, PathType::Temp).toStdString();
    if (QFileInfo(rollback_path).exists())
        std::filesystem::remove(rollback_path);
    else
        std::filesystem::create_directories(rollback_path.parent_path());
    std::filesystem::rename(path.toStdString(), rollback_path);
    std::filesystem::rename(temp_path, path.toStdString());
}

template<typename T> void Fernanda::saveConfig(QString group, QString valueName, T value)
{
    QSettings ini(config, QSettings::IniFormat);
    ini.beginGroup(group);
    ini.setValue(valueName, value);
    ini.endGroup();
}

const QVariant Fernanda::loadConfig(QString group, QString valueName, bool preventRandomStrAsTrue)
{
    if (!QFile(config).exists()) return -1;
    QSettings ini(config, QSettings::IniFormat);
    ini.beginGroup(group);
    if (!ini.childKeys().contains(valueName)) return -1;
    auto val = ini.value(valueName);
    if (val.isNull()) return -1;
    if (preventRandomStrAsTrue && val != "true" && val != "false") return -1;

    return val;
}

void Fernanda::loadResourceConfig(QList<QAction*> actions, QString group, QString valueName, QString fallback)
{
    for (auto action : actions)
    {
        auto resource = loadConfig(group, valueName).toString();
        if (resource == action->data())
        {
            action->setChecked(true);
            return;
        }
    }
    for (auto action : actions)
    {
        if (action->data() == fallback)
        {
            action->setChecked(true);
            return;
        }
    }
    actions.first()->setChecked(true);
}

void Fernanda::loadMiscConfigsOnOpen()
{
    auto window_position = loadConfig("window", "position");
    if (window_position == -1 || !window_position.canConvert<QRect>())
        setGeometry(0, 0, 1000, 666);
    else
        setGeometry(window_position.toRect());

    auto window_max = loadConfig("window", "max", true);
    if (window_max != -1)
        if (window_max.toBool())
            setWindowState(Qt::WindowState::WindowMaximized);

    // I do not understand what is happening with testing QByteArray entry in .ini file with empty string or garbage string. It seems to somehow still remember the needed splitter position even when the ini value is blank or random characters...
    auto splitter_position = loadConfig("window", "splitter");
    if (splitter_position == -1 || !window_position.canConvert<QByteArray>())
        splitter->setSizes(QList<int>{ 166, 834 });
    else
        splitter->restoreState(splitter_position.toByteArray());

    auto font_slider_position = loadConfig("editor", "font_size");
    if (font_slider_position.toInt() < 8 || font_slider_position.toInt() > 64)
        fontSlider->setValue(14);
    else
        fontSlider->setValue(font_slider_position.toInt());

    // AOT status cannot be loaded before or close to MainWindow geometry, otherwise geometry will not load correctly.
    auto aot_check = loadConfig("window", "aot", true);
    if (aot_check == -1)
        aot->setChecked(false);
    else
        aot->setChecked(aot_check.toBool());
}

bool Fernanda::loadProjectDataOnOpen()
{
    auto default_projects = loadConfig("data", "default_projects");
    auto default_projects_path = default_projects.toString();
    auto current_project = loadConfig("data", "current_project");
    auto current_project_path = current_project.toString();

    if (default_projects != -1 && QDir(default_projects_path).exists())
    {
        if (current_project != -1 && QDir(current_project_path).exists())
        {
            auto is_child = checkChildStatus(default_projects_path, current_project_path);
            if (!is_child) return false;
            defaultProjectsFolder = default_projects_path;
            currentProject = current_project_path;
            pane->setup(currentProject);
            checkTempsOnOpen();
            return true;
        }
        defaultProjectsFolder = default_projects_path;
    }
    return false;
}

void Fernanda::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    saveConfig<QRect>("window", "position", geometry());
}

void Fernanda::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
    saveConfig<QRect>("window", "position", geometry());
}

void Fernanda::menuToggles(QWidget* widget, QString group, QString value, bool checked)
{
    widget->setVisible(checked);
    saveConfig<bool>(group, value, checked);
}

void Fernanda::loadMenuToggles(QAction* action, QString group, QString value, bool fallback)
{
    auto state = loadConfig(group, value, true);
    if (state == -1)
    {
        // For some reason, start-up toggles need to be toggled twice. It seems as though they are not connected when initially toggled

        action->setChecked(!fallback); // whyyyyyyyyy
        action->setChecked(fallback);
    }
    else
    {
        action->setChecked(!state.toBool()); // whyyyyyyyyy
        action->setChecked(state.toBool());
    }
}

void Fernanda::togglePosAndCounts(bool& globalBool, QString group, QString value, bool checked)
{
    globalBool = checked;
    saveConfig<bool>(group, value, checked);
    updatePositions();
    updateCounters();
}

void Fernanda::startColorBar(ColorScheme scheme)
{
    if (scheme == ColorScheme::None) return;
    setColorBarStyle(scheme);
    auto* bar_fill = new QTimeLine(125, this);
    connect(bar_fill, &QTimeLine::frameChanged, colorBar, &QProgressBar::setValue);
    bar_fill->setFrameRange(0, 100);
    colorBar->show();
    barTimer->start(1000);
    bar_fill->start();
}

void Fernanda::setColorBarStyle(ColorScheme scheme)
{
    QString style_sheet;
    if (scheme == ColorScheme::Red)
        style_sheet = readFile(":/themes/bar/red.qss");
    else if (scheme == ColorScheme::Green)
        style_sheet = readFile(":/themes/bar/green.qss");
    else if (scheme == ColorScheme::StartUp)
        style_sheet = readFile(":/themes/bar/start_up.qss");
    colorBar->setStyleSheet(style_sheet);
}

void Fernanda::clearAll(ClearType type)
{
    dirtyFiles.clear();
    pane->paneDelegate->dirtyIndexes.clear();
    textEditor->cursorPositions.clear();
    if (type == ClearType::Full)
    {
        cleanText = "";
        textEditor->clear();
        pathDisplay->setText("");
        pane->clearTuples();
    }
    textEditorTextChanged();
}

bool Fernanda::alert(ColorScheme scheme, QString message, QString optButton, void (Fernanda::* optButtonAction)(), QString optReplaceMainButton, QString optReplaceTitle)
{
    QMessageBox alert;
    auto title = tr("Hey!");
    if (!optReplaceTitle.isEmpty())
        title = tr(optReplaceTitle.toUtf8());
    alert.setWindowTitle(title);
    alert.setText(message);
    QAbstractButton* ok = nullptr;
    auto ok_text = tr("Okay");
    if (!optReplaceMainButton.isEmpty())
        ok_text = tr(optReplaceMainButton.toUtf8());
    ok = alert.addButton(ok_text, QMessageBox::AcceptRole);
    if (!optButton.isEmpty())
    {
        QAbstractButton* option = alert.addButton(tr(optButton.toUtf8()), QMessageBox::AcceptRole);
        alert.exec();
        if (alert.clickedButton() == option)
        {
            std::invoke(optButtonAction, *this);
            // (this->*optButtonAction)(); // lol
            startColorBar(scheme);
            return true;
        }
        return false;
    }
    alert.exec();
    startColorBar(scheme);
    return false;
}

void Fernanda::chooseProjectDataOnOpen()
{
    if (!defaultProjectsFolder.isEmpty())
    {
        auto alert_option_taken = alert(ColorScheme::Green, "Open an existing project, or create a new one", "Create a new project (WIP)", &Fernanda::fileMenuNewProject, "Open an existing project");
        if (!alert_option_taken)
            fileMenuOpenProject();
    }
    else
    {
        helpMenuSetProjectsDir();
    }
}

void Fernanda::textEditorTextChanged() // Move to TextEditor?
{
    // Bug: if you load a blank file first (or startup blank) while lineNumberArea is on, then then initially the editor's viewport will not account for the lineNumberArea width, and they'll overlap, until something else is loaded or lineNumberArea is toggled off/on, in which case the width will be accounted for, even for only one line.
    auto& current_file_index = std::get<1>(pane->currentFile);
    auto& current_file = std::get<0>(pane->currentFile);
    if (current_file.isEmpty())
    {
        overlay->show();
        textEditor->setReadOnly(true);
    }
    else
    {
        overlay->hide();
        textEditor->setReadOnly(false);
        auto current_text = textEditor->toPlainText();
        if (current_text != cleanText)
        {
            if (!pane->paneDelegate->dirtyIndexes.contains(current_file_index))
            {
                pane->paneDelegate->dirtyIndexes << current_file_index;
                pane->refresh();
            }
            if (!dirtyFiles.contains(current_file))
            {
                dirtyFiles << current_file;
                displayPath();
            }
        }
        else
        {
            pane->paneDelegate->dirtyIndexes.removeAll(current_file_index);
            dirtyFiles.removeAll(current_file);
            pane->refresh();
            displayPath();
        }
    }
}

void Fernanda::fileMenuOpenProject()
{
    if (!defaultProjectsFolder.isEmpty())
    {
        auto project = QFileDialog::getExistingDirectory(this, "Open a project", defaultProjectsFolder);
        if (project.isEmpty())
        {
            startColorBar(ColorScheme::Red);
            return;
        }
        auto is_child = checkChildStatus(defaultProjectsFolder, project);
        if (!is_child)
        {
            auto opt_taken = alert(ColorScheme::Red, "A project must be a subfolder of your Default Projects directory!", "Choose a new Default Projects directory...", &Fernanda::helpMenuSetProjectsDir);
            if (opt_taken) return;
            fileMenuOpenProject();
            return;
        }
        if (currentProject == project)
        {
            startColorBar(ColorScheme::Red);
            return;
        }
        fileMenuSaveAll();
        clearAll(ClearType::Full);
        currentProject = project;
        saveConfig<QString>("data", "current_project", currentProject);
        pane->setup(currentProject);
        startColorBar(ColorScheme::Green);
    }
    else
    {
        helpMenuSetProjectsDir();
    }
}

void Fernanda::fileMenuNewProject()
{
    // WIP
}

void Fernanda::fileMenuNew()
{
    if (!currentProject.isEmpty())
    {
        auto file_name = QFileDialog::getSaveFileName(this, tr("Create a new file"), currentProject, tr("Plain text file (*.txt)"));
        if (file_name.isEmpty())
        {
            startColorBar(ColorScheme::Red);
            return;
        }
        if (!QFile(file_name).exists())
        {
            writeFile("", file_name);
            startColorBar(ColorScheme::Green);
        }
    }
}

void Fernanda::fileMenuNewSubfolder()
{
    if (!currentProject.isEmpty())
    {
        auto subfolder_name = QFileDialog::getSaveFileName(this, tr("Create a new subfolder"), currentProject, tr("Folder"));
        if (subfolder_name.isEmpty())
        {
            startColorBar(ColorScheme::Red);
            return;
        }
        if (!QDir(subfolder_name).exists())
        {
            QDir().mkdir(subfolder_name);
            startColorBar(ColorScheme::Green);
        }
    }
}

void Fernanda::fileMenuSave()
{
    auto& current_file = std::get<0>(pane->currentFile);
    if (!current_file.isEmpty())
    {
        auto text = textEditor->toPlainText();
        if (cleanText != text)
        {
            auto temp_path = pathMaker(current_file, PathType::Temp);
            tempSave(text, temp_path);
            textEditor->rememberCursorPositions(temp_path);
            swap(current_file);
            cleanText = readFile(current_file);
            textEditorTextChanged();
            startColorBar(ColorScheme::Green);
        }
    }
}

void Fernanda::fileMenuSaveAll()
{
    if (!pane->paneDelegate->dirtyIndexes.isEmpty() || !dirtyFiles.isEmpty())
    {
        fileMenuSave();
        for (auto& item : dirtyFiles)
            swap(item);
        clearAll(ClearType::Partial);
        startColorBar(ColorScheme::Green);
    }
}

void Fernanda::helpMenuSetProjectsDir()
{
    alert(ColorScheme::None, "In order to use Fernanda, you need to set a default directory for your projects.");

    auto documents = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    std::filesystem::path tmp_docs = documents.toStdString();
    auto fernanda_docs = tmp_docs / "Fernanda";
    std::filesystem::create_directories(fernanda_docs);
    auto path = QString::fromStdString(fernanda_docs.string());
    auto default_projects = QFileDialog::getExistingDirectory(this, "Set a Default Projects directory", path);

    if (defaultProjectsFolder.isEmpty() && default_projects.isEmpty())
    {
        startColorBar(ColorScheme::Red);
        helpMenuSetProjectsDir();
        return;
    }
    if (default_projects.isEmpty())
    {
        startColorBar(ColorScheme::Red);
        return;
    }
    if (defaultProjectsFolder == default_projects)
    {
        startColorBar(ColorScheme::Red);
        return;
    }
    defaultProjectsFolder = default_projects;
    saveConfig<QString>("data", "default_projects", defaultProjectsFolder);

    alert(ColorScheme::None, "Now select a project folder.", "Generate a sample project", &Fernanda::helpMenuMakeSample);
    fileMenuOpenProject();
}

void Fernanda::helpMenuMakeSample()
{
    makeSample(defaultProjectsFolder);
}

void Fernanda::helpMenuMakeSampleUdRc()
{
    auto could_create = createSampleThemesAndFonts(userData);
    if (!could_create)
    {
        alert(ColorScheme::Red, "One or more sample custom themes and/or fonts still exist in your data folder.");
        return;
    }
    alert(ColorScheme::Green, "You'll need to restart in order to see the sample custom themes and fonts added to the View submenus.");
}

void Fernanda::helpMenuAbout()
{
    QMessageBox about;
    about.setWindowTitle("About");
    //QPixmap icon(":/icons/fernanda.png");
    //about.setIconPixmap(icon.scaledToHeight(64, Qt::FastTransformation));
    about.setText("Fernanda is a personal project and a work-in-progress.");
    QAbstractButton* ok = about.addButton(tr("Okay"), QMessageBox::AcceptRole);
    about.exec();
}

void Fernanda::ifPreviousFileIsFile(QString path)
{
    auto prev_text = textEditor->toPlainText();
    auto prev_file_temp_path = pathMaker(path, PathType::Temp);
    tempSave(prev_text, prev_file_temp_path);
    textEditor->rememberCursorPositions(prev_file_temp_path);
}

void Fernanda::ifPathDoesNotEqualPrevPath(QString path)
{
    auto temp_path = pathMaker(path, PathType::Temp);
    auto text = tempOpen(path, temp_path);
    cleanText = readFile(path);
    textEditor->setPlainText(text);
    textEditor->setFocus();
    if (QFileInfo(temp_path).exists())
        textEditor->restoreCursorPositions(temp_path);
}

void Fernanda::setWindowStyle()
{
    if (auto selection = windowThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = createStyleSheetFromTheme(
            readFile(":/themes/window.qss"),
            readFile(theme_path)
        );
        setStyleSheet(style_sheet);
        saveConfig<QString>("window", "wintheme", theme_path);
    }
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = createStyleSheetFromTheme(
            readFile(":/themes/editor.qss"),
            readFile(theme_path)
        );
        overlay->setStyleSheet(style_sheet);
        textEditor->setStyleSheet(style_sheet);
        underlay->setStyleSheet(style_sheet);

        auto theme_cursor = readFile(theme_path);
        QRegularExpressionMatch match_cursor = QRegularExpression("(@cursorColor; = )(.*)(;$)").match(theme_cursor);
        QString cursor_color = match_cursor.captured(2);

        textEditor->cursorColorHex = cursor_color;
        saveConfig<QString>("editor", "theme", theme_path);
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
        saveConfig<QString>("editor", "font", selection->data().toString());
    }
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
    saveConfig("window", "aot", checked);
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

void Fernanda::setTabStop()
{
    if (auto selection = tabStops->checkedAction(); selection != nullptr)
    {
        auto distance = selection->data().toInt();
        textEditor->setTabStopDistance(distance);
        saveConfig<QVariant>("editor", "tab", distance);
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
        saveConfig<QVariant>("editor", "wrap", mode);
    }
}

// fernanda.cpp, fernanda