#include "fernanda.h"

Fernanda::Fernanda(QWidget *parent)
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
    if (has_full_projects_data == false)
    {
        QTimer::singleShot(1250, this, &Fernanda::chooseProjectDataOnOpen);
    }
    // Bug: After start-up, the first folder clicked in Pane is slow to load. After that, it's fine?
}

void Fernanda::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (wasInitialized == true || event->spontaneous())
    {
        return;
    }
    textEditorTextChanged();
    QTimer::singleShot(1000, this, &Fernanda::startUpColorBar);
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
    connect(splitter, &QSplitter::splitterMoved, this, &Fernanda::splitterResize);
    connect(pane, &Pane::previousFileIsFile, this, &Fernanda::ifPreviousFileIsFile);
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, &Fernanda::ifPathDoesNotEqualPrevPath);
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, &Fernanda::startAutoSaveTimer);
    connect(textEditor, &TextEditor::askNavPrevious, pane, &Pane::navPrevious);
    connect(textEditor, &TextEditor::askNavNext, pane, &Pane::navNext);
    connect(textEditor, &TextEditor::askFontSliderZoom, this, &Fernanda::zoomFontSlider);
    connect(this, &Fernanda::sendLineHighlightToggle, textEditor, &TextEditor::toggleLineHight);
    connect(this, &Fernanda::sendLineNumberAreaToggle, textEditor, &TextEditor::toggleLineNumberArea);
    connect(this, &Fernanda::sendTabStop, textEditor, &TextEditor::setTabStop);
    connect(this, &Fernanda::sendWrapMode, textEditor, &TextEditor::setWrapMode);
    connect(autoSaveTimer, &QTimer::timeout, this, &Fernanda::autoSave);
    connect(pane, &Pane::changePathDisplay, this, &Fernanda::displayPath);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::textEditorTextChanged);
    connect(textEditor, &TextEditor::cursorPositionChanged, this, &Fernanda::updatePositions);
    connect(textEditor, &TextEditor::textChanged, this, &Fernanda::updateCounters);
    connect(aot, &QPushButton::toggled, this, &Fernanda::aotToggled);
    connect(barTimer, &QTimer::timeout, this, &Fernanda::colorBarTimerOver);
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
            {
                setWindowState(Qt::WindowState::WindowMaximized);
                // I would actually prefer to save *any* window state when closeEvent is triggered (before moving to active) and save to ini, then return to that state if close is canceled.
            }
            event->ignore();
            return;
        }
    }
    clearTempFiles();
    event->accept();
}

void Fernanda::displayPath()
{
    auto& full_index_path = get<1>(pane->selectedIndex);
    fs::path index_path = full_index_path.toStdString();
    auto index_root = index_path.root_name();
    auto index_drive_letter = QString::fromStdString(index_root.string()) + "\\>";
    auto index_name = index_path.filename();
    auto _index_name = QString::fromStdString(index_name.string());

    auto& full_file_path = get<0>(pane->currentFile);
    fs::path file_path = full_file_path.toStdString();
    auto file_root = file_path.root_name();
    auto file_drive_letter = QString::fromStdString(file_root.string()) + "\\>";
    auto file_name = file_path.filename();
    auto _file_name = QString::fromStdString(file_name.string());
    
    QString italics_1;
    QString italics_2;
    if (dirtyFiles.contains(full_file_path))
    {
        italics_1 = "<i>";
        italics_2 = "*</i>";
    }
    else
    {
        italics_1 = "";
        italics_2 = "";
    }
    QString par_1;
    QString par_2;
    if (!full_file_path.isEmpty())
    {
        par_1 = " (";
        par_2 = ")";
    }
    else
    {
        par_1 = "";
        par_2 = "";
    }

    if (index_path == file_path)
    {
        pathDisplay->setText(file_drive_letter + italics_1 + _file_name + italics_2);
    }
    else
    {
        pathDisplay->setText(index_drive_letter + _index_name + par_1 + italics_1 + _file_name + italics_2 + par_2);
    }
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
    QList<tuple<QString, QString>> win_theme_list = resourceIterator(":/themes/window/", "*.fernanda_wintheme", ResourceType::WindowTheme);
    windowThemes = createMenuToggles(win_theme_list, &Fernanda::setWindowStyle);

    QList<tuple<QString, QString>> editor_theme_list = resourceIterator(":/themes/editor/", "*.fernanda_theme", ResourceType::EditorTheme);
    editorThemes = createMenuToggles(editor_theme_list, &Fernanda::setEditorStyle);

    QList<tuple<QString, QString>> font_list = resourceIterator(":/fonts/", "*.ttf", ResourceType::Font);
    editorFonts = createMenuToggles(font_list, &Fernanda::setEditorFont);

    auto* font_size_label = new QAction(tr("&Set editor font size:"), this);
    font_size_label->setStatusTip(tr("Set editor font size"));
    font_size_label->setEnabled(false);

    auto* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(64);

    QList<tuple<QString, QString>> tab_list = {
        tuple<QString, QString>("20", "20 px"),
        tuple<QString, QString>("40", "40 px"),
        tuple<QString, QString>("60", "60 px"),
        tuple<QString, QString>("80", "80 px"),
    };
    tabStops = createMenuToggles(tab_list, &Fernanda::setTabStop);

    QList<tuple<QString, QString>> wrap_list = {
        tuple<QString, QString>("NoWrap", "No wrap"),
        tuple<QString, QString>("WordWrap", "Wrap at word boundaries"),
        tuple<QString, QString>("WrapAnywhere", "Wrap anywhere"),
        tuple<QString, QString>("WrapAt", "Wrap at word boundaries or anywhere")
    };
    wrapModes = createMenuToggles(wrap_list, &Fernanda::setWrapMode);

    auto* toggle_aot = new QAction(tr("&Toggle always-on-top button"), this);
    toggle_aot->setStatusTip(tr("Toggle always-on-top button"));
    connect(toggle_aot, &QAction::toggled, this, &Fernanda::toggleAOT);
    toggle_aot->setCheckable(true);

    auto* toggle_pane = new QAction(tr("&Toggle file pane"), this);
    toggle_pane->setStatusTip(tr("Toggle file pane"));
    connect(toggle_pane, &QAction::toggled, this, &Fernanda::togglePane);
    toggle_pane->setCheckable(true);

    auto* toggle_extra_scrolls = new QAction(tr("&Toggle scroll previous and next buttons"), this);
    toggle_extra_scrolls->setStatusTip(tr("Toggle scroll previous and next buttons"));
    connect(toggle_extra_scrolls, &QAction::toggled, this, &Fernanda::toggleScrollPrevAndNext);
    toggle_extra_scrolls->setCheckable(true);

    auto* toggle_statusbar = new QAction(tr("&Toggle statusbar"), this);
    toggle_statusbar->setStatusTip(tr("Toggle statusbar"));
    connect(toggle_statusbar, &QAction::toggled, this, &Fernanda::toggleStatusbar);
    toggle_statusbar->setCheckable(true);

    auto* toggle_line_highlight = new QAction(tr("&Toggle current line highlight"), this);
    toggle_line_highlight->setStatusTip(tr("Toggle current line highlight"));
    connect(toggle_line_highlight, &QAction::toggled, this, &Fernanda::toggleLineHighlight);
    toggle_line_highlight->setCheckable(true);

    auto* toggle_key_filters = new QAction(tr("&Toggle key filters"), this);
    toggle_key_filters->setStatusTip(tr("Toggle key filters"));
    connect(toggle_key_filters, &QAction::toggled, this, &Fernanda::toggleKeyFilters);
    toggle_key_filters->setCheckable(true);

    auto* toggle_line_numbers = new QAction(tr("&Toggle line number area"), this);
    toggle_line_numbers->setStatusTip(tr("Toggle line number area"));
    connect(toggle_line_numbers, &QAction::toggled, this, &Fernanda::toggleLineNumberArea);
    toggle_line_numbers->setCheckable(true);

    auto* toggle_line_pos = new QAction(tr("&Toggle line position"), this);
    toggle_line_pos->setStatusTip(tr("Toggle line position"));
    connect(toggle_line_pos, &QAction::toggled, this, &Fernanda::toggleLinePos);
    toggle_line_pos->setCheckable(true);

    auto* toggle_col_pos = new QAction(tr("&Toggle column position"), this);
    toggle_col_pos->setStatusTip(tr("Toggle column position"));
    connect(toggle_col_pos, &QAction::toggled, this, &Fernanda::toggleColPos);
    toggle_col_pos->setCheckable(true);

    auto* toggle_line_count = new QAction(tr("&Toggle line count"), this);
    toggle_line_count->setStatusTip(tr("Toggle line count"));
    connect(toggle_line_count, &QAction::toggled, this, &Fernanda::toggleLineCount);
    toggle_line_count->setCheckable(true);

    auto* toggle_word_count = new QAction(tr("&Toggle word count"), this);
    toggle_word_count->setStatusTip(tr("Toggle word count"));
    connect(toggle_word_count, &QAction::toggled, this, &Fernanda::toggleWordCount);
    toggle_word_count->setCheckable(true);

    auto* toggle_char_count = new QAction(tr("&Toggle character count"), this);
    toggle_char_count->setStatusTip(tr("Toggle character count"));
    connect(toggle_char_count, &QAction::toggled, this, &Fernanda::toggleCharCount);
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
    connect(fontSlider, &QSlider::valueChanged, this, &Fernanda::setEditorFontSize);

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

const QList<tuple<QString, QString>> Fernanda::resourceIterator(QString path, QString ext, ResourceType type)
{
    QList<tuple<QString, QString>> dataAndLabels;
    QDirIterator assets(path, QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
    if (QDir(userData).exists())
    {
        QDirIterator user_assets(QString::fromStdString(userData.string()), QStringList() << ext, QDir::Files, QDirIterator::Subdirectories);
        while (user_assets.hasNext())
        {
            user_assets.next();

            auto label = resourceNameCap(user_assets.filePath());

            if (type == ResourceType::Font)
            {
                dataAndLabels << tuple<QString, QString>(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(user_assets.filePath())).at(0), label);
            }
            else
            {
                dataAndLabels << tuple<QString, QString>(user_assets.filePath(), label);
            }
        }
    }
    while (assets.hasNext())
    {
        assets.next();

        auto label = resourceNameCap(assets.filePath());

        if (type == ResourceType::Font)
        {
            dataAndLabels << tuple<QString, QString>(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(assets.filePath())).at(0), label);
        }
        else
        {
            dataAndLabels << tuple<QString, QString>(assets.filePath(), label);
        }
    }
    sort(dataAndLabels.begin(), dataAndLabels.end());
    return dataAndLabels;
}

const QString Fernanda::resourceNameCap(QString path)
{
    fs::path file_path = path.toStdString();
    auto stem = file_path.stem();
    auto name = QString::fromStdString(stem.string());
    auto name_capped = name.left(1).toUpper() + name.mid(1);
    return name_capped;
}

QActionGroup* Fernanda::createMenuToggles(QList<tuple<QString, QString>>& itemAndLabel, void (Fernanda::* slot)())
{
    auto* group = new QActionGroup(this);
    for (auto& item : itemAndLabel)
    {
        auto& data = get<0>(item);
        auto label = get<1>(item).toUtf8();
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
    userData = fs::path(QDir::homePath().toStdString()) / dataFolderName.toStdString();
    auto config_path = userData / configName.toStdString();
    config = QString::fromStdString(config_path.string());
    activeTemp = userData / string(".active_temp");
    backup = userData / string("backup");
    rollback = backup / string(".rollback");
    QList<fs::path> data_folders = { userData, activeTemp, backup, rollback };
    for (auto& folder : data_folders)
    {
        if (!QDir(folder).exists())
        {
            fs::create_directory(folder);
        }
    }
}

void Fernanda::clearTempFiles()
{
    for (auto& item : fs::directory_iterator(activeTemp))
    {
        fs::remove_all(item);
    }
}

const QString Fernanda::readFile(QString path)
{
    QString text;
    QFile file(path);
    if (file.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        text = in.readAll();
        file.close();
    }
    return text;
}

const QString Fernanda::pathMaker(QString path, PathType type)
{
    fs::path data_folder;
    QString prefix;
    QString ext;

    if (type == PathType::Backup)
    {
        //
    }
    else if (type == PathType::Original)
    {
        return path;
    }
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

    auto proj_relative = fs::relative(path.toStdString(), defaultProjectsFolder.toStdString());
    fs::path proj_relative_parent = prefix.toStdString() + proj_relative.parent_path().string();
    fs::path file_name = prefix.toStdString() + proj_relative.filename().replace_extension(ext.toStdString()).string();
    auto derived_path = data_folder / proj_relative_parent / file_name;
    derived_path.make_preferred();

    return QString::fromStdString(derived_path.string());
}

void Fernanda::writeFile(QString text, QString path)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << text;
        file.close();
    }
}

void Fernanda::readWriteSampleFiles(QString path, fs::path subfolder)
{
    auto text = readFile(path);
    fs::path file_name = path.toStdString();
    auto new_path = subfolder / file_name.filename();
    auto new_path_q = QString::fromStdString(new_path.string());
    writeFile(text, new_path_q);
}

bool Fernanda::createUdThemesAndFonts()
{
    auto font_fs = userData / "Merriweather.ttf";
    auto editor_theme_fs = userData / "sample.fernanda_theme";
    auto win_theme_fs = userData / "sample.fernanda_wintheme";
    auto font = QString::fromStdString(font_fs.string());
    auto editor_theme = QString::fromStdString(editor_theme_fs.string());
    auto win_theme = QString::fromStdString(win_theme_fs.string());
    if (QFile(font).exists() || QFile(editor_theme).exists() || QFile(win_theme).exists())
    {
        return false;
    }
    QFile::copy(":/sample/Merriweather.ttf", font);
    QFile::copy(":/sample/sample.fernanda_theme", editor_theme);
    QFile::copy(":/sample/sample.fernanda_wintheme", win_theme);
    QFile(font).setPermissions(QFile::WriteUser);
    QFile(editor_theme).setPermissions(QFile::WriteUser);
    QFile(win_theme).setPermissions(QFile::WriteUser);
    return true;
}

void Fernanda::tempSave(QString text, QString path)
{
    fs::path temp_path = path.toStdString();
    fs::create_directories(temp_path.parent_path());
    writeFile(text, path);
}

const QString Fernanda::tempOpen(QString path, QString tempPath)
{
    QFileInfo temp_info(tempPath);
    if (temp_info.exists())
    {
        return readFile(tempPath);
    }
    else
    {
        return readFile(path);
    }
}

void Fernanda::swap(QString path)
{
    fs::path rollback_path = pathMaker(path, PathType::Rollback).toStdString();
    fs::path temp_path = pathMaker(path, PathType::Temp).toStdString();
    if (QFileInfo(rollback_path).exists())
    {
        fs::remove(rollback_path);
    }
    else
    {
        fs::create_directories(rollback_path.parent_path());
    }
    fs::rename(path.toStdString(), rollback_path);
    fs::rename(temp_path, path.toStdString());
}

const QString Fernanda::createStyleSheetFromTheme(QString styleSheet, QString themeSheet)
{
    auto style_sheet = readFile(styleSheet);
    auto theme_sheet = readFile(themeSheet);
    QRegularExpressionMatchIterator matches = QRegularExpression("(@.*\\n?)").globalMatch(theme_sheet);
    while (matches.hasNext())
    {
        QRegularExpressionMatch match = matches.next();
        if (match.hasMatch())
        {
            QString variable = match.captured(0).replace(QRegularExpression("(\\s=.*;)"), "");
            QString value = match.captured(0).replace(QRegularExpression("(@.*=\\s)"), "");
            style_sheet.replace(QRegularExpression(variable), value);
        }
    }
    return style_sheet;
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
    if (QFile(config).exists())
    {
        QSettings ini(config, QSettings::IniFormat);
        ini.beginGroup(group);
        if (ini.childKeys().contains(valueName))
        {
            auto val = ini.value(valueName).toString();
            if (val.isEmpty())
            {
                return -1;
            }
            if (preventRandomStrAsTrue == true && val != "true" && val != "false")
            {
                return -1;
            }
            return ini.value(valueName);
        }
        return -1;
    }
    return -1;
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
    // Bug: window position is not being recalled on load (it's defaulting to the rect given below, every time). It must be receiving -1 somehow, but I'm not sure how. Other toggles/values are being recalled correctly.
    // Tried: removing canConvert option below
    // Adding "false" to the optional arg for loadConfig
    auto window_position = loadConfig("window", "position");
    if (window_position == -1 || !window_position.canConvert<QRect>())
    {
        setGeometry(0, 0, 1000, 666);
    }
    else
    {
        setGeometry(window_position.toRect());
    }

    auto window_max = loadConfig("window", "max", true);
    if (window_max != -1)
    {
        if (window_max.toBool() == true)
        {
            setWindowState(Qt::WindowState::WindowMaximized);
        }
    }

    // I do not understand what is happening with testing QByteArray entry in .ini file with empty string or garbage string. It seems to somehow still remember the needed splitter position even when the ini value is blank or random characters...
    auto splitter_position = loadConfig("window", "splitter");
    if (splitter_position == -1 || !window_position.canConvert<QByteArray>())
    {
        splitter->setSizes(QList<int>{ 166, 834 });
    }
    else
    {
        splitter->restoreState(splitter_position.toByteArray());
    }

    auto font_slider_position = loadConfig("editor", "font_size");
    if (font_slider_position.toInt() < 8 || font_slider_position.toInt() > 64)
    {
        fontSlider->setValue(14);
    }
    else
    {
        fontSlider->setValue(font_slider_position.toInt());
    }

    // AOT status cannot be loaded before or close to MainWindow geometry, otherwise geometry will not load correctly.
    auto aot_check = loadConfig("window", "aot", true);
    if (aot_check == -1)
    {
        aot->setChecked(false);
    }
    else
    {
        aot->setChecked(aot_check.toBool());
    }
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
            if (is_child == false)
            {
                return false;
            }
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
    if (scheme == ColorScheme::None)
    {
        return;
    }
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
    {
        style_sheet = readFile(":/themes/bar/red.qss");
    }
    else if (scheme == ColorScheme::Green)
    {
        style_sheet = readFile(":/themes/bar/green.qss");
    }
    else if (scheme == ColorScheme::StartUp)
    {
        style_sheet = readFile(":/themes/bar/start_up.qss");
    }
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

bool Fernanda::checkChildStatus(QString possibleParent, QString possibleChild)
{
    fs::path parent = possibleParent.toStdString();
    fs::path child = possibleChild.toStdString();
    fs::path child_parent = child.parent_path();
    if (child_parent.make_preferred() == parent.make_preferred())
    {
        return true;
    }
    return false;
}

bool Fernanda::alert(ColorScheme scheme, QString message, QString optButton, void (Fernanda::* optButtonAction)(), QString optReplaceMainButton, QString optReplaceTitle)
{
    QMessageBox alert;
    if (!optReplaceTitle.isEmpty())
    {
        alert.setWindowTitle(optReplaceTitle);
    }
    else
    {
        alert.setWindowTitle("Hey!");
    }
    alert.setText(message);
    QAbstractButton* ok = nullptr;
    if (!optReplaceMainButton.isEmpty())
    {
        ok = alert.addButton(tr(optReplaceMainButton.toUtf8()), QMessageBox::AcceptRole);
    }
    else
    {
        ok = alert.addButton(tr("Okay"), QMessageBox::AcceptRole);
    }
    if (!optButton.isEmpty())
    {
        QAbstractButton* option = alert.addButton(tr(optButton.toUtf8()), QMessageBox::AcceptRole);
        alert.exec();
        if (alert.clickedButton() == option)
        {
            invoke(optButtonAction, *this);
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
        auto option_taken = alert(ColorScheme::Green, "Open an existing project, or create a new one", "Create a new project (WIP)", &Fernanda::fileMenuNewProject, "Open an existing project");
        if (option_taken == false)
        {
            fileMenuOpenProject();
        }
    }
    else
    {
        helpMenuSetProjectsDir();
    }
}

void Fernanda::startUpColorBar()
{
    startColorBar(ColorScheme::StartUp);
}

void Fernanda::textEditorTextChanged() // Move to TextEditor?
{
    // Bug: if you load a blank file first (or startup blank) while lineNumberArea is on, then then initially the editor's viewport will not account for the lineNumberArea width, and they'll overlap, until something else is loaded or lineNumberArea is toggled off/on, in which case the width will be accounted for, even for only one line.
    auto& current_file_index = get<1>(pane->currentFile);
    auto& current_file = get<0>(pane->currentFile);
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
        if (is_child == false)
        {
            auto opt_taken = alert(ColorScheme::Red, "A project must be a subfolder of your Default Projects directory!", "Choose a new Default Projects directory...", &Fernanda::helpMenuSetProjectsDir);
            if (opt_taken == true)
            {
                return;
            }
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
    auto& current_file = get<0>(pane->currentFile);
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
        {
            swap(item);
        }
        clearAll(ClearType::Partial);
        startColorBar(ColorScheme::Green);
    }
}

void Fernanda::helpMenuSetProjectsDir()
{
    alert(ColorScheme::None, "In order to use Fernanda, you need to set a default directory for your projects.");

    auto documents = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    fs::path tmp_docs = documents.toStdString();
    auto fernanda_docs = tmp_docs / "Fernanda";
    fs::create_directories(fernanda_docs);
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
    fs::path candide;
    fs::path projects_folder = defaultProjectsFolder.toStdString();
    candide = projects_folder / "Candide";
    auto candide_subfolder_1 = candide / string("Chapters 1-10");
    auto candide_subfolder_2 = candide / string("Chapters 11-20");
    auto candide_subfolder_3 = candide / string("Chapters 21-30");
    auto candide_subfolder_4 = candide / string("Misc");
    QList<fs::path> sample_folders = { candide, candide_subfolder_1, candide_subfolder_2, candide_subfolder_3, candide_subfolder_4 };
    for (auto& folder : sample_folders)
    {
        fs::create_directory(folder);
    }
    // I tried doing the following programmatically, and it was noticably slower than just doing it line-by-line.
    QString sample_parent_path = ":/sample/Candide/";
    QStringList candide_files_1 = {
        sample_parent_path + "Chapter 1.txt",
        sample_parent_path + "Chapter 2.txt",
        sample_parent_path + "Chapter 3.txt",
        sample_parent_path + "Chapter 4.txt",
        sample_parent_path + "Chapter 5.txt",
        sample_parent_path + "Chapter 6.txt",
        sample_parent_path + "Chapter 7.txt",
        sample_parent_path + "Chapter 8.txt",
        sample_parent_path + "Chapter 9.txt",
        sample_parent_path + "Chapter 10.txt"
    };
    QStringList candide_files_2 = {
        sample_parent_path + "Chapter 11.txt",
        sample_parent_path + "Chapter 12.txt",
        sample_parent_path + "Chapter 13.txt",
        sample_parent_path + "Chapter 14.txt",
        sample_parent_path + "Chapter 15.txt",
        sample_parent_path + "Chapter 16.txt",
        sample_parent_path + "Chapter 17.txt",
        sample_parent_path + "Chapter 18.txt",
        sample_parent_path + "Chapter 19.txt",
        sample_parent_path + "Chapter 20.txt"
    };
    QStringList candide_files_3 = {
        sample_parent_path + "Chapter 21.txt",
        sample_parent_path + "Chapter 22.txt",
        sample_parent_path + "Chapter 23.txt",
        sample_parent_path + "Chapter 24.txt",
        sample_parent_path + "Chapter 25.txt",
        sample_parent_path + "Chapter 26.txt",
        sample_parent_path + "Chapter 27.txt",
        sample_parent_path + "Chapter 28.txt",
        sample_parent_path + "Chapter 29.txt",
        sample_parent_path + "Chapter 30.txt"
    };
    QStringList candide_files_4 = {
        sample_parent_path + "Footnotes.txt",
        sample_parent_path + "Project Gutenberg License.txt",
        sample_parent_path + "Title Page and Introduction.txt"
    };
    for (auto& sample_path : candide_files_1)
    {
        readWriteSampleFiles(sample_path, candide_subfolder_1);
    }
    for (auto& sample_path : candide_files_2)
    {
        readWriteSampleFiles(sample_path, candide_subfolder_2);
    }
    for (auto& sample_path : candide_files_3)
    {
        readWriteSampleFiles(sample_path, candide_subfolder_3);
    }
    for (auto& sample_path : candide_files_4)
    {
        readWriteSampleFiles(sample_path, candide_subfolder_4);
    }
}

void Fernanda::helpMenuMakeSampleUdRc()
{
    auto existed = createUdThemesAndFonts();
    if (existed == false)
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
    {
        textEditor->restoreCursorPositions(temp_path);
    }
}

void Fernanda::setWindowStyle()
{
    if (auto selection = windowThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = createStyleSheetFromTheme(":/themes/window.qss", theme_path);
        setStyleSheet(style_sheet);
        saveConfig<QString>("window", "wintheme", theme_path);
    }
}

void Fernanda::setEditorStyle()
{
    if (auto selection = editorThemes->checkedAction(); selection != nullptr)
    {
        auto theme_path = selection->data().toString();
        auto style_sheet = createStyleSheetFromTheme(":/themes/editor.qss", theme_path);
        overlay->setStyleSheet(style_sheet);
        textEditor->setStyleSheet(style_sheet);
        underlay->setStyleSheet(style_sheet);

        auto theme_cursor = readFile(theme_path);
        QRegularExpressionMatch match_cursor = QRegularExpression("(@cursorColor; = )(.*)(;$)").match(theme_cursor);
        QString cursor_color = match_cursor.captured(2);

        textEditor->setCursorColor(cursor_color);
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

void Fernanda::setEditorFontSize(int value)
{
    setEditorFont();
    saveConfig<int>("editor", "font_size", value);
}

void Fernanda::splitterResize()
{
    saveConfig<QByteArray>("window", "splitter", splitter->saveState());
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

void Fernanda::startAutoSaveTimer()
{
    autoSaveTimer->start(30000);
}

void Fernanda::autoSave()
{
    auto& path = get<0>(pane->currentFile);
    ifPreviousFileIsFile(path);
}

void Fernanda::toggleAOT(bool checked)
{
    menuToggles(aot, "window", "aot_button_visible", checked);
}

void Fernanda::togglePane(bool checked)
{
    menuToggles(pane, "window", "pane_visible", checked);
}

void Fernanda::toggleScrollPrevAndNext(bool checked)
{
    menuToggles(textEditor->scrollPrevious, "editor", "scroll_previous_visible", checked);
    menuToggles(textEditor->scrollNext, "editor", "scroll_next_visible", checked);
}

void Fernanda::toggleStatusbar(bool checked)
{
    menuToggles(statusBar(), "window", "statusbar_visible", checked);
}

void Fernanda::toggleLineHighlight(bool checked)
{
    sendLineHighlightToggle(checked);
    saveConfig<bool>("editor", "line_highlight", checked);
}

void Fernanda::toggleKeyFilters(bool checked)
{
    textEditor->hasKeyFilters = checked;
    saveConfig<bool>("editor", "key_filters", checked);
}

void Fernanda::toggleLineNumberArea(bool checked)
{
    sendLineNumberAreaToggle(checked);
    saveConfig<bool>("editor", "line_number_area", checked);
}

void Fernanda::updatePositions()
{
    const auto line_pos = textEditor->textCursor().blockNumber();
    const auto col_pos = textEditor->textCursor().positionInBlock();
    QStringList elements;
    if (linePos == true)
    {
        elements << "ln " + QString::number(line_pos + 1);
    }
    if (colPos == true)
    {
        elements << "col " + QString::number(col_pos + 1);
    }
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
    if (lineCount == true)
    {
        elements << QString::number(line_count) + " lines";
    }
    if (wordCount == true)
    {
        elements << QString::number(word_count - 1) + " words"; // - 1, whyyyyy
        // The above always displays at least 1, even when editor is blank?
    }
    if (charCount == true)
    {
        elements << QString::number(char_count) + " chars";
    }
    auto counter_display = elements.join(", ");
    counters->setText(counter_display);
}

void Fernanda::toggleLinePos(bool checked)
{
    togglePosAndCounts(linePos, "window", "show_line_position", checked);
}

void Fernanda::toggleColPos(bool checked)
{
    togglePosAndCounts(colPos, "window", "show_column_position", checked);
}

void Fernanda::toggleLineCount(bool checked)
{
    togglePosAndCounts(lineCount, "window", "show_line_count", checked);
}

void Fernanda::toggleWordCount(bool checked)
{
    togglePosAndCounts(wordCount, "window", "show_word_count", checked);
}

void Fernanda::toggleCharCount(bool checked)
{
    togglePosAndCounts(charCount, "window", "show_character_count", checked);
}

void Fernanda::setTabStop()
{
    if (auto selection = tabStops->checkedAction(); selection != nullptr)
    {
        auto distance = selection->data().toInt();
        sendTabStop(distance);
        saveConfig<QVariant>("editor", "tab", distance);
    }
}

void Fernanda::setWrapMode()
{
    if (auto selection = wrapModes->checkedAction(); selection != nullptr)
    {
        auto mode = selection->data().toString();
        if (mode == "NoWrap")
        {
            sendWrapMode(QTextOption::NoWrap);
        }
        else if (mode == "WordWrap")
        {
            sendWrapMode(QTextOption::WordWrap);
        }
        else if (mode == "WrapAnywhere")
        {
            sendWrapMode(QTextOption::WrapAnywhere);
        }
        else if (mode == "WrapAt")
        {
            sendWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        }
        saveConfig<QVariant>("editor", "wrap", mode);
    }
}

void Fernanda::colorBarTimerOver()
{
    colorBar->hide();
    colorBar->reset();
}

void Fernanda::zoomFontSlider(bool zoomDirection)
{
    if (zoomDirection == true)
    {
        fontSlider->setValue(fontSlider->value() + 2);
    }
    else
    {
        fontSlider->setValue(fontSlider->value() - 2);
    }
}

// fernanda.cpp, fernanda