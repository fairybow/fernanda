// editor.cpp, fernanda

#include "editor.h"

TextEditor::TextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    cursorBlink->setTimerType(Qt::VeryCoarseTimer);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    addScrollBarWidget(scrollUp, Qt::AlignTop);
    addScrollBarWidget(scrollPrevious, Qt::AlignTop);
    addScrollBarWidget(scrollNext, Qt::AlignBottom);
    addScrollBarWidget(scrollDown, Qt::AlignBottom);
    scrollUp->setAutoRepeat(true);
    scrollDown->setAutoRepeat(true);
    scrollUp->setText(Uni::ico(Uni::Ico::TriangleUp));
    scrollPrevious->setText(Uni::ico(Uni::Ico::TriangleUp));
    scrollNext->setText(Uni::ico(Uni::Ico::TriangleDown));
    scrollDown->setText(Uni::ico(Uni::Ico::TriangleDown));
    for (auto& button : { scrollUp, scrollPrevious, scrollNext, scrollDown })
        button->setMinimumHeight(30);
    setObjectName("textEditor");
    lineNumberArea->setObjectName("lineNumberArea");
    horizontalScrollBar()->setObjectName("hScrollBar");
    verticalScrollBar()->setObjectName("vScrollBar");
    scrollUp->setObjectName("scrollUp");
    scrollPrevious->setObjectName("scrollPrevious");
    scrollNext->setObjectName("scrollNext");
    scrollDown->setObjectName("scrollDown");
    connections();
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    auto block = firstVisibleBlock();
    auto block_number = block.blockNumber();
    auto top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    auto bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            auto number = QString::number(block_number + 1);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++block_number;
    }
}

int TextEditor::lineNumberAreaWidth()
{
    if (lineNumberArea->isVisible())
    {
        auto digits = 1;
        auto max = qMax(1, blockCount());
        while (max >= 10)
        {
            max /= 10;
            ++digits;
        }
        auto space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
        return space;
    }
    return 0;
}

TextEditor::Action TextEditor::handleKeySwap(QString oldKey, QString newKey)
{
    if (newKey == nullptr)
    {
        clear();
        setReadOnly(true);
        cursorPositions.clear();
        return Action::Cleared;
    }
    if (oldKey == newKey)
    {
        setFocus();
        return Action::None;
    }
    if (isReadOnly())
        setReadOnly(false);
    if (oldKey != nullptr)
        storeCursors(oldKey);
    return Action::AcceptNew;
}

void TextEditor::handleTextSwap(QString key, QString text)
{
    clear();
    setPlainText(text);
    recallCursors(key);
    recallUndoStacks(key);
    setFocus();
}

int TextEditor::selectedLineCount()
{
    auto cursor = textCursor();
    if (!cursor.hasSelection()) return 1;
    return cursor.selectedText().count(Uni::regex(Uni::Re::ParagraphSeparator)) + 1;
}

void TextEditor::scrollNavClicked(Scroll direction)
{
    if (!askHasProject()) return;
    auto early_return = false;
    switch (direction) {
    case Scroll::Next:
        if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->maximum())
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
            early_return = true;
        }
        break;
    case Scroll::Previous:
        if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->minimum())
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMinimum);
            early_return = true;
        }
        break;
    }
    if (early_return) return;
    (direction == Scroll::Next)
        ? askNavNext()
        : askNavPrevious();
}

void TextEditor::setFont(QString font, int size)
{
    QFont q_font(font);
    q_font.setStyleStrategy(QFont::PreferAntialias);
    q_font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    q_font.setPointSize(size);
    QPlainTextEdit::setFont(q_font);
    lineNumberArea->setFont(q_font);
}

void TextEditor::toggleLineHighlight(bool checked)
{
    hasLineHighlight = checked;
    highlightCurrentLine();
}

void TextEditor::toggleKeyfilter(bool checked)
{
    hasKeyfilter = checked;
}

void TextEditor::toggleLineNumberArea(bool checked)
{
    lineNumberArea->setVisible(checked);
    updateLineNumberAreaWidth(0);
}

void TextEditor::toggleScrolls(bool checked)
{
    scrollUp->setVisible(checked);
    scrollDown->setVisible(checked);
}

void TextEditor::toggleExtraScrolls(bool checked)
{
    scrollPrevious->setVisible(checked);
    scrollNext->setVisible(checked);
}

void TextEditor::toggleBlockCursor(bool checked)
{
    hasBlockCursor = checked;
    cursorPositionChanged();
}

void TextEditor::toggleCursorBlink(bool checked)
{
    hasCursorBlink = checked;
    startBlinker();
}

void TextEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    auto c_r = contentsRect();
    lineNumberArea->setGeometry(QRect(c_r.left(), c_r.top(), lineNumberAreaWidth(), c_r.height()));
}

void TextEditor::paintEvent(QPaintEvent* event)
{
    QPlainTextEdit::paintEvent(event);
    QPainter painter(viewport());
    auto cur_char = currentChar();
    auto rect = reshapeCursor(cur_char);
    painter.fillRect(rect, recolorCursor());
    if (!cur_char.isNull() && hasBlockCursor)
    {
        painter.setPen(recolorCursor(true));
        painter.drawText(rect, cur_char);
    }
}

void TextEditor::wheelEvent(QWheelEvent* event)
{
    (event->modifiers() == Qt::ControlModifier)
        ? (event->angleDelta().y() > 0)
            ? askFontSliderZoom(Zoom::In)
            : askFontSliderZoom(Zoom::Out)
        : QPlainTextEdit::wheelEvent(event);
    event->accept();
}

void TextEditor::keyPressEvent(QKeyEvent* event)
{
    QTextCursor cursor = textCursor();
    if (shortcutFilter(event))
    {
        event->ignore();
        return;
    }
    if (!hasKeyfilter)
    {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }
    auto chars = proximalChars();
    cursor.beginEditBlock();
    keyPresses(keyfilter->filter(event, chars));
    cursor.endEditBlock();
    if (cursor.atEnd() && verticalScrollBar()->sliderPosition() != verticalScrollBar()->maximum())
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
}

void TextEditor::keyPresses(QVector<QKeyEvent*> events)
{
    for (auto& event : events)
        QPlainTextEdit::keyPressEvent(event);
}

const QChar TextEditor::currentChar()
{
    auto text = textCursor().block().text();
    auto cur_pos = textCursor().positionInBlock();
    if (cur_pos < text.size())
        return text.at(cur_pos);
    return QChar();
}

const Keyfilter::ProximalChars TextEditor::proximalChars()
{
    auto text = textCursor().block().text();
    auto cur_pos = textCursor().positionInBlock();
    auto result = Keyfilter::ProximalChars{};
    if (cur_pos < text.size())
        result.current = text.at(cur_pos);
    if (cur_pos > 0)
        result.previous = text.at(static_cast<qsizetype>(cur_pos) - 1);
    if (cur_pos > 1)
        result.beforeLast = text.at(static_cast<qsizetype>(cur_pos) - 2);
    return result;
}

bool TextEditor::shortcutFilter(QKeyEvent* event)
{
    if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        if (event->key() == Qt::Key_C)
        {
            quoteWrap(event);
            return true;
        }
    }
    return false;
}

void TextEditor::quoteWrap(QKeyEvent* event)
{
    QKeyEvent backspace{ QKeyEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier };
    QKeyEvent quote{ QKeyEvent::KeyPress, Qt::Key_QuoteDbl, Qt::NoModifier, QString('"') };
    QKeyEvent right{ QKeyEvent::KeyPress, Qt::Key_Right, Qt::NoModifier };
    auto cursor = textCursor();
    auto text = cursor.block().text();
    cursor.beginEditBlock();
    if (cursor.hasSelection())
    {
        auto selection = cursor.selectedText();
        auto start_pos = cursor.selectionStart();
        auto end_pos = cursor.selectionEnd();
        cursor.setPosition(start_pos);
        setTextCursor(cursor);
        QPlainTextEdit::keyPressEvent(&quote);
        cursor.setPosition(end_pos);
        setTextCursor(cursor);
        if (selection.endsWith(" "))
            keyPresses({ &right, &backspace, &quote });
        else if (selection.end()->isNull())
            keyPresses({ &right, &quote });
        else
            QPlainTextEdit::keyPressEvent(&quote);
    }
    else
    {
        cursor.movePosition(QTextCursor::StartOfBlock);
        setTextCursor(cursor);
        QPlainTextEdit::keyPressEvent(&quote);
        cursor.movePosition(QTextCursor::EndOfBlock);
        setTextCursor(cursor);
        if (text.endsWith(" "))
            keyPresses({ &backspace, &quote });
        else
            QPlainTextEdit::keyPressEvent(&quote);
    }
    cursor.endEditBlock();
}

void TextEditor::connections()
{
    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
    connect(scrollNext, &QPushButton::clicked, this, [&]() { scrollNavClicked(Scroll::Next); });
    connect(scrollPrevious, &QPushButton::clicked, this, [&]() { scrollNavClicked(Scroll::Previous); });
    connect(this, &TextEditor::startBlinker, this, [&]()
        {
            if (!hasCursorBlink) return;
            cursorBlink->start(200);
        });
    connect(this, &TextEditor::cursorPositionChanged, this, [&]()
        {
            if (textCursor().hasSelection() || !hasCursorBlink) return;
            cursorVisible = true;
            startBlinker();
        });
    connect(cursorBlink, &QTimer::timeout, this, [&]()
        {
            cursorVisible = !cursorVisible;
            startBlinker();
        });
    connect(scrollUp, &QPushButton::clicked, this, [&]()
        {
            for (auto i = 2; i > 0; --i)
                verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        });
    connect(scrollDown, &QPushButton::clicked, this, [&]()
        {
            for (auto i = 2; i > 0; --i)
                verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        });
}

const QRect TextEditor::reshapeCursor(QChar currentChar)
{
    if (hasBlockCursor)
    {
        QFontMetrics metrics(font());
        (currentChar.isNull())
            ? setCursorWidth(metrics.averageCharWidth())
            : setCursorWidth(metrics.horizontalAdvance(currentChar));
    }
    else
        setCursorWidth(2);
    auto result = cursorRect(textCursor());
    setCursorWidth(0);
    return result;
}

const QColor TextEditor::recolorCursor(bool under)
{
    QColor result;
    if (!cursorVisible && hasCursorBlink)
        result = QColor(0, 0, 0, 0);
    else
    {
        (under)
            ? result = QColor(cursorUnderColorHex)
            : result = QColor(cursorColorHex);
    }
    return result;
}

const QColor TextEditor::highlight()
{
    QColor result;
    (hasLineHighlight)
        ? result = QColor(255, 255, 255, 30)
        : result = QColor(0, 0, 0, 0);
    return result;
}

void TextEditor::storeCursors(QString key)
{
    for (auto& item : cursorPositions)
        if (key == item.key)
            cursorPositions.removeAll(item);
    cursorPositions << CursorPositions{
        key,
        QTextCursor(textCursor()).position(),
        QTextCursor(textCursor()).anchor()
    };
}

void TextEditor::recallCursors(QString key)
{
    for (auto& item : cursorPositions)
        if (key == item.key)
        {
            auto cursor(textCursor());
            auto cursor_pos = item.position;
            auto anchor_pos = item.anchor;
            if (cursor_pos == anchor_pos)
                cursor.setPosition(cursor_pos);
            else
            {
                cursor.setPosition(anchor_pos, QTextCursor::MoveAnchor);
                cursor.setPosition(cursor_pos, QTextCursor::KeepAnchor);
            }
            setTextCursor(cursor);
            cursorPositions.removeAll(item);
            break;
        }
}

void TextEditor::recallUndoStacks(QString key) // WIP
{
    //
}

void TextEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::highlightCurrentLine()
{
    QVector<QTextEdit::ExtraSelection> extra_selections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(highlight());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extra_selections.append(selection);
    }
    setExtraSelections(extra_selections);
}

void TextEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    (dy)
        ? lineNumberArea->scroll(0, dy)
        : lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

// editor.cpp, fernanda