#include "fernanda.h"

Fernanda::Fernanda(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    nameObjects();
    layoutObjects();
    makeConnections();
    createUserData(userDataName);
    loadConfigOnOpen();
    pane->setup(currentProject);
    makeMenuBar();
    textEditorTextChanged();
    onOpen();
    hotkeys();
}

void Fernanda::nameObjects() 
{
    setWindowTitle(fernandaName);
    splitter->setObjectName("splitter");
    pane->setObjectName("pane");
    textEditor->setObjectName("textEditor");
    pathDisplay->setObjectName("pathDisplay");
    positions->setObjectName("positions");
    counters->setObjectName("counters");
    aot->setObjectName("aot");
}

void Fernanda::layoutObjects()
{
    ui.mainToolBar->deleteLater();
    splitter->addWidget(pane);
    splitter->addWidget(textEditor);
    splitter->setCollapsible(0, true);
    splitter->setCollapsible(1, false);
    setCentralWidget(splitter);
    statusBar()->addPermanentWidget(pathDisplay, 1);
    statusBar()->addPermanentWidget(positions, 0);
    statusBar()->addPermanentWidget(counters, 0);
    statusBar()->addPermanentWidget(aot, 0);
    aot->setCheckable(true);
    aot->setText("\U0001F4CC");
    pane->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    textEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    splitter->setStretchFactor(1, 100);
    statusBar()->setMaximumHeight(22);
}

void Fernanda::makeConnections()
{
    connect(splitter, &QSplitter::splitterMoved, this, &Fernanda::splitterResize);
    connect(pane, &Pane::previousFileIsFile, this, &Fernanda::ifPreviousFileIsFile);
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, &Fernanda::ifPathDoesNotEqualPrevPath);
    connect(pane, &Pane::pathDoesNotEqualPrevPath, this, &Fernanda::startAutoSaveTimer);
    connect(textEditor, &TextEditor::askNavPrevious, pane, &Pane::navPrevious);
    connect(textEditor, &TextEditor::askNavNext, pane, &Pane::navNext);
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
}

void Fernanda::onOpen()
{
    if (!QDir(activeTemp).isEmpty())
    {
        auto message = QMessageBox::question(this, fernandaName, fernandaName + " may not have properly quit last time, and unsaved changes are available. Would you like to recover them?", QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if (message == QMessageBox::No)
        {
            clearTempFiles();
        }
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
            event->ignore();
            return;
        }
    }
    clearTempFiles();
    event->accept();
}

void Fernanda::hotkeys()
{
    //
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
}

void Fernanda::makeFileMenu()
{
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
    file->addAction(save);
    file->addAction(save_all);
    file->addSeparator();
    file->addAction(quit);
}

void Fernanda::makeViewMenu()
{
    QList<tuple<QString, QString>> win_theme_list = resourceIterator(":/themes/window/", "*.fernanda_wintheme", Resource::WindowTheme);
    windowThemes = createSelectionMenu(win_theme_list, &Fernanda::setWindowStyle);

    QList<tuple<QString, QString>> editor_theme_list = resourceIterator(":/themes/editor/", "*.fernanda_theme", Resource::EditorTheme);
    editorThemes = createSelectionMenu(editor_theme_list, &Fernanda::setEditorStyle);

    QList<tuple<QString, QString>> font_list = resourceIterator(":/fonts/", "*.ttf", Resource::Font);
    editorFonts = createSelectionMenu(font_list, &Fernanda::setEditorFont);

    QWidgetAction* font_size = new QWidgetAction(this);
    font_size->setDefaultWidget(fontSlider);
    fontSlider->setMinimum(8);
    fontSlider->setMaximum(72);

    QList<tuple<QString, QString>> tab_list = {
        tuple<QString, QString>("20", "20 px"),
        tuple<QString, QString>("40", "40 px"),
        tuple<QString, QString>("60", "60 px"),
        tuple<QString, QString>("80", "80 px"),
    };
    tabStops = createSelectionMenu(tab_list, &Fernanda::setTabStop);

    QList<tuple<QString, QString>> wrap_list = {
        tuple<QString, QString>("NoWrap", "No wrap"),
        tuple<QString, QString>("WordWrap", "Wrap at word boundaries"),
        tuple<QString, QString>("WrapAnywhere", "Wrap anywhere"),
        tuple<QString, QString>("WrapAt", "Wrap at word boundaries or anywhere")
    };
    wrapModes = createSelectionMenu(wrap_list, &Fernanda::setWrapMode);

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

const QList<tuple<QString, QString>> Fernanda::resourceIterator(QString path, QString ext, Resource type)
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

            if (type == Resource::Font)
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

        if (type == Resource::Font)
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

QActionGroup* Fernanda::createSelectionMenu(QList<tuple<QString, QString>> itemAndLabel, void (Fernanda::* fx)())
{
    auto* group = new QActionGroup(this);
    for (auto& item : itemAndLabel)
    {
        auto& data = get<0>(item);
        auto label = get<1>(item).toUtf8();
        auto* action = new QAction(tr(label), this);
        action->setStatusTip(tr(label));
        action->setData(data);
        connect(action, &QAction::toggled, this, fx);
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
    sample = userData / string("sample");
    candide = sample / string("Candide");
    if (!QDir(sample).exists() || !QDir(candide).exists())
    {
        createSample();
    }
    if (currentProject.isEmpty())
    {
        defaultProjectsFolder = QString::fromStdString(sample.string());
        currentProject = QString::fromStdString(candide.string());
    }
    createUdThemesAndFonts(userData);
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

const QString Fernanda::pathMaker(QString path, ChangeTo key)
{
    fs::path data_folder;
    QString prefix;
    QString ext;

    if (key == ChangeTo::BackupPath)
    {
        //
    }
    else if (key == ChangeTo::OriginalPath)
    {
        return path;
    }
    else if (key == ChangeTo::RollbackPath)
    {
        data_folder = rollback;
        prefix = "";
        ext = ".fernanda_bak";
    }
    else if (key == ChangeTo::TempPath)
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
    }
}

void Fernanda::createSample()
{
    candideSubfolder_1 = candide / string("Chapters 1-10");
    candideSubfolder_2 = candide / string("Chapters 11-20");
    candideSubfolder_3 = candide / string("Chapters 21-30");
    candideSubfolder_4 = candide / string("Misc");
    QList<fs::path> sample_folders = { sample, candide, candideSubfolder_1, candideSubfolder_2, candideSubfolder_3, candideSubfolder_4 };
    for (auto& folder : sample_folders)
    {
        fs::create_directory(folder);
    }
    createSampleFiles();
}

void Fernanda::createSampleFiles()
{
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
        readWriteSampleFiles(sample_path, candideSubfolder_1);
    }
    for (auto& sample_path : candide_files_2)
    {
        readWriteSampleFiles(sample_path, candideSubfolder_2);
    }
    for (auto& sample_path : candide_files_3)
    {
        readWriteSampleFiles(sample_path, candideSubfolder_3);
    }
    for (auto& sample_path : candide_files_4)
    {
        readWriteSampleFiles(sample_path, candideSubfolder_4);
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

void Fernanda::createUdThemesAndFonts(fs::path path)
{
    auto font_fs = path / "Merriweather-Regular.ttf";
    auto editor_theme_fs = path / "test.fernanda_theme";
    auto win_theme_fs = path / "test.fernanda_wintheme";

    auto font = QString::fromStdString(font_fs.string());
    auto editor_theme = QString::fromStdString(editor_theme_fs.string());
    auto win_theme = QString::fromStdString(win_theme_fs.string());

    QFile::copy(":/test/Merriweather-Regular.ttf", font);
    QFile::copy(":/test/test.fernanda_theme", editor_theme);
    QFile::copy(":/test/test.fernanda_wintheme", win_theme);

    QFile(font).setPermissions(QFile::WriteUser);
    QFile(editor_theme).setPermissions(QFile::WriteUser);
    QFile(win_theme).setPermissions(QFile::WriteUser);
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
    fs::path rollback_path = pathMaker(path, ChangeTo::RollbackPath).toStdString();
    fs::path temp_path = pathMaker(path, ChangeTo::TempPath).toStdString();
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

template<typename T> void Fernanda::saveConfig(QString group, QString value, T x)
{
    QSettings ini(config, QSettings::IniFormat);
    ini.beginGroup(group);
    ini.setValue(value, x);
    ini.endGroup();
}

const QVariant Fernanda::loadConfig(QString group, QString value)
{
    if (QFile(config).exists())
    {
        QSettings ini(config, QSettings::IniFormat);
        ini.beginGroup(group);
        if (ini.childKeys().contains(value))
        {
            return ini.value(value);
        }
        return 0;
    }
    return 0;
}

void Fernanda::loadResourceConfig(QList<QAction*> actions, QString group, QString value, QString fallback)
{
    for (auto action : actions)
    {
        auto resource = loadConfig(group, value).toString();
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

void Fernanda::loadConfigOnOpen()
{
    auto aot_check = loadConfig("window", "aot");
    if (aot_check == 0)
    {
        aot->setChecked(false);
    }
    else
    {
        aot->setChecked(aot_check.toBool());
    }
    auto window_position = loadConfig("window", "position");
    if (window_position == 0)
    {
        setGeometry(0, 0, 1000, 666);
    }
    else
    {
        setGeometry(window_position.toRect());
    }
    auto window_max = loadConfig("window", "max");
    if (window_max.toBool() == true)
    {
        setWindowState(Qt::WindowState::WindowMaximized);
    }
    auto splitter_position = loadConfig("window", "splitter");
    if (splitter_position == 0)
    {
        splitter->setSizes(QList<int>{ 166, 834 });
    }
    else
    {
        splitter->restoreState(splitter_position.toByteArray());
    }
    auto font_slider_position = loadConfig("editor", "font_size");
    if (font_slider_position.toInt() < 8 || font_slider_position.toInt() > 72)
    {
        fontSlider->setValue(14);
    }
    else
    {
        fontSlider->setValue(font_slider_position.toInt());
    }
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
    auto state = loadConfig(group, value);
    if (state == 0)
    {
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

void Fernanda::textEditorTextChanged() // move to texteditor
{
    auto& current_file_index = get<1>(pane->currentFile);
    auto& current_file = get<0>(pane->currentFile);
    if (current_file.isEmpty())
    {
        textEditor->setReadOnly(true);
    }
    else
    {
        textEditor->setReadOnly(false);
        auto current_text = textEditor->toPlainText();
        if (!cleanText.isEmpty())
        {
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
}

void Fernanda::fileMenuSave()
{
    auto& current_file = get<0>(pane->currentFile);
    if (!current_file.isEmpty())
    {
        auto text = textEditor->toPlainText();
        auto temp_path = pathMaker(current_file, ChangeTo::TempPath);
        tempSave(text, temp_path);
        textEditor->rememberCursorPositions(temp_path);
        swap(current_file);
        cleanText = readFile(current_file);
        textEditorTextChanged();
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
        dirtyFiles.clear();
        pane->paneDelegate->dirtyIndexes.clear();
        textEditor->cursorPositions.clear();
        textEditorTextChanged();
    }
}

void Fernanda::ifPreviousFileIsFile(QString path)
{
    auto prev_text = textEditor->toPlainText();
    auto prev_file_temp_path = pathMaker(path, ChangeTo::TempPath);
    tempSave(prev_text, prev_file_temp_path);
    textEditor->rememberCursorPositions(prev_file_temp_path);
}

void Fernanda::ifPathDoesNotEqualPrevPath(QString path)
{
    auto temp_path = pathMaker(path, ChangeTo::TempPath);
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
        textEditor->setStyleSheet(style_sheet);

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
    autoSaveTimer->start(60000);
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
        elements << QString::number(word_count - 1) + " words"; // whyyyy
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

// fernanda.cpp, fernanda