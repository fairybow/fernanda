#include "texteditor.h"

TextEditor::TextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    nameObjects();
    layoutObjects();
    makeConnections();
    //updateLineNumberAreaWidth(0);
    //highlightCurrentLine();
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(255, 255, 255, 15));
    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    auto top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    auto bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            auto number = QString::number(blockNumber + 1);
            painter.setPen(QColor(255, 255, 255, 30));
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
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

void TextEditor::rememberCursorPositions(QString path)
{
    for (auto& entry : cursorPositions)
    {
        if (path == get<0>(entry))
        {
            cursorPositions.removeAll(entry);
        }
    }
    auto cursor_position = QTextCursor(textCursor()).position();
    auto anchor_position = QTextCursor(textCursor()).anchor();
    cursorPositions << tuple<QString, int, int>(path, cursor_position, anchor_position);
}

void TextEditor::restoreCursorPositions(QString path)
{
    auto cursor(textCursor());
    auto cursor_position = 0;
    auto anchor_position = 0;
    for (auto& entry : cursorPositions)
    {
        if (get<0>(entry) == path)
        {
            cursor_position = get<1>(entry);
            anchor_position = get<2>(entry);
            cursorPositions.removeAll(entry);
        }
    }
    if (cursor_position == anchor_position)
    {
        cursor.setPosition(cursor_position);
        setTextCursor(cursor);
    }
    else
    {
        cursor.setPosition(anchor_position, QTextCursor::MoveAnchor);
        cursor.setPosition(cursor_position, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    }
}

void TextEditor::lineNumberAreaFont(int size, QString fontName)
{
    QFont font(fontName);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    font.setPointSize(size);
    lineNumberArea->setFont(font);
}

void TextEditor::setCursorColor(QString hex)
{
    cursorColorHex = hex;
}

void TextEditor::toggleLineHight(bool checked)
{
    hasLineHighlight = checked;
    highlightCurrentLine();
}

void TextEditor::toggleLineNumberArea(bool checked)
{
    lineNumberArea->setVisible(checked);
    updateLineNumberAreaWidth(0);
}

void TextEditor::setTabStop(int pixels)
{
    setTabStopDistance(pixels);
}

void TextEditor::setWrapMode(QTextOption::WrapMode mode)
{
    setWordWrapMode(mode);
}

void TextEditor::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    auto cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEditor::nameObjects()
{
    lineNumberArea->setObjectName("lineNumberArea");
    horizontalScrollBar()->setObjectName("hScrollBar");
    verticalScrollBar()->setObjectName("vScrollBar");
    scrollPrevious->setObjectName("scrollPrevious");
    scrollUp->setObjectName("scrollUp");
    scrollDown->setObjectName("scrollDown");
    scrollNext->setObjectName("scrollNext");
}

void TextEditor::layoutObjects()
{
    addScrollBarWidget(scrollUp, Qt::AlignTop);
    addScrollBarWidget(scrollPrevious, Qt::AlignTop);
    addScrollBarWidget(scrollNext, Qt::AlignBottom);
    addScrollBarWidget(scrollDown, Qt::AlignBottom);
    scrollUp->setAutoRepeat(true);
    scrollDown->setAutoRepeat(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    const QString mdl2 = "Segoe MDL2 Assets";
    scrollUp->setFont(mdl2);
    scrollUp->setText("\U0000E70E");
    scrollPrevious->setFont(mdl2);
    scrollPrevious->setText("\U0000E96D");
    scrollNext->setFont(mdl2);
    scrollNext->setText("\U0000E96E");
    scrollDown->setFont(mdl2);
    scrollDown->setText("\U0000E70D");

    scrollPrevious->setMinimumHeight(25);
    scrollUp->setMinimumHeight(30);
    scrollDown->setMinimumHeight(30);
    scrollNext->setMinimumHeight(25);
}

void TextEditor::makeConnections()
{
    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
    connect(scrollPrevious, &QPushButton::clicked, this, &TextEditor::scrollPreviousClicked);
    connect(scrollUp, &QPushButton::clicked, this, &TextEditor::scrollUpClicked);
    connect(scrollDown, &QPushButton::clicked, this, &TextEditor::scrollDownClicked);
    connect(scrollNext, &QPushButton::clicked, this, &TextEditor::scrollNextClicked);
}

void TextEditor::paintEvent(QPaintEvent* event)
{
    QPlainTextEdit::paintEvent(event);
    if (hasFocus())
    {
        const QRect rect = cursorRect(textCursor());
        QPainter painter(viewport());
        painter.fillRect(rect, cursorColor());
    }
}

const QColor TextEditor::cursorColor()
{
    QColor cursor_color(cursorColorHex);
    cursor_color.setAlpha(180);
    return cursor_color;
}

void TextEditor::keyPressEvent(QKeyEvent* event) // lord jesus how do i make this better?
{
    if (hasKeyFilters == true)
    {
        auto cursor = QPlainTextEdit::textCursor();
        cursor.beginEditBlock();
        if (event->modifiers() == (Qt::ShiftModifier | Qt::AltModifier) && event->key() == Qt::Key_Q || event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) && event->key() == Qt::Key_QuoteDbl)
        {
            auto cursor_position = cursor.position();
            if (!cursor.atBlockEnd())
            {
                QPlainTextEdit::keyPressEvent(&quote);
                cursor.movePosition(QTextCursor::EndOfBlock);
                QPlainTextEdit::setTextCursor(cursor);
                auto cursor_position_end = cursor.position();
                auto current_text = QPlainTextEdit::toPlainText();
                QChar previous_char;
                if (cursor_position_end <= current_text.size() && cursor_position_end != 0)
                {
                    previous_char = current_text.at(static_cast<qsizetype>(cursor_position_end) - 1);
                }
                if (previous_char == ' ')
                {
                    QPlainTextEdit::keyPressEvent(&backspace);
                    QPlainTextEdit::keyPressEvent(&quote);
                    cursor.setPosition(cursor_position);
                    QPlainTextEdit::setTextCursor(cursor);
                }
                else
                {
                    QPlainTextEdit::keyPressEvent(&quote);
                    cursor.setPosition(cursor_position);
                    QPlainTextEdit::setTextCursor(cursor);
                }
            }
        }
        else if (event->key() == Qt::Key_BraceLeft)
        {
            QPlainTextEdit::keyPressEvent(event);
            QPlainTextEdit::keyPressEvent(&braceRight);
            QPlainTextEdit::keyPressEvent(&left);
        }
        else if (event->key() == Qt::Key_BraceRight)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar current_char;
            if (cursor_position < current_text.size())
            {
                current_char = current_text.at(cursor_position);
            }
            if (current_char == '}')
            {
                QPlainTextEdit::keyPressEvent(&right);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_BracketLeft)
        {
            QPlainTextEdit::keyPressEvent(event);
            QPlainTextEdit::keyPressEvent(&bracketRight);
            QPlainTextEdit::keyPressEvent(&left);
        }
        else if (event->key() == Qt::Key_BracketRight)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar current_char;
            if (cursor_position < current_text.size())
            {
                current_char = current_text.at(cursor_position);
            }
            if (current_char == ']')
            {
                QPlainTextEdit::keyPressEvent(&right);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_Comma || event->key() == Qt::Key_Exclam || event->key() == Qt::Key_Question)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar previous_char;
            QChar char_before_last;
            std::unordered_set<QChar> chars_close_space_after{ '}', ']', ')', '"' };
            if (cursor_position <= current_text.size() && cursor_position != 0)
            {
                previous_char = current_text.at(static_cast<qsizetype>(cursor_position) - 1);
            }
            if (cursor_position <= current_text.size() && cursor_position != 0 && cursor_position != 1)
            {
                char_before_last = current_text.at(static_cast<qsizetype>(cursor_position) - 2);
            }
            if (chars_close_space_after.count(char_before_last) > 0 && previous_char == ' ')
            {
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(&left);
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&deleteKey);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_Minus)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar previous_char;
            if (cursor_position <= current_text.size() && cursor_position != 0)
            {
                previous_char = current_text.at(static_cast<qsizetype>(cursor_position) - 1);
            }
            if (previous_char == '-')
            {
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&emDash);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_ParenLeft)
        {
            QPlainTextEdit::keyPressEvent(event);
            QPlainTextEdit::keyPressEvent(&parenRight);
            QPlainTextEdit::keyPressEvent(&left);
        }
        else if (event->key() == Qt::Key_ParenRight)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar current_char;
            if (cursor_position < current_text.size())
            {
                current_char = current_text.at(cursor_position);
            }
            if (current_char == ')')
            {
                QPlainTextEdit::keyPressEvent(&right);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_Period)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar previous_char;
            QChar char_before_last;
            QChar char_before_before_last;
            std::unordered_set<QChar> chars_close_space_after{ '}', ']', ')', '"' };
            if (cursor_position <= current_text.size() && cursor_position != 0)
            {
                previous_char = current_text.at(static_cast<qsizetype>(cursor_position) - 1);
            }
            if (cursor_position <= current_text.size() && cursor_position != 0 && cursor_position != 1)
            {
                char_before_last = current_text.at(static_cast<qsizetype>(cursor_position) - 2);
            }
            if (cursor_position <= current_text.size() && cursor_position != 0 && cursor_position != 1 && cursor_position != 2)
            {
                char_before_before_last = current_text.at(static_cast<qsizetype>(cursor_position) - 3);
            }
            if (chars_close_space_after.count(char_before_last) > 0 && previous_char == ' ')
            {
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(&left);
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&deleteKey);
            }
            else if (char_before_before_last == ' ' && char_before_last == '.' && previous_char == '.')
            {
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
            }
            else if (char_before_before_last != ' ' && char_before_last == '.' && previous_char == '.')
            {
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&space);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else if (event->key() == Qt::Key_QuoteDbl)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar current_char;
            if (cursor_position < current_text.size())
            {
                current_char = current_text.at(cursor_position);
            }
            if (current_char == '"')
            {
                QPlainTextEdit::keyPressEvent(&right);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(event);
                QPlainTextEdit::keyPressEvent(&left);
            }
        }
        else if (event->key() == Qt::Key_Space)
        {
            auto cursor_position = cursor.position();
            auto current_text = QPlainTextEdit::toPlainText();
            QChar current_char;
            QChar previous_char;
            QChar char_before_last;
            std::unordered_set<QChar> chars_space_skip{ '}', ']', ',', '!', ')', '.', '?', '"' };
            if (cursor_position < current_text.size() && cursor_position != 0)
            {
                current_char = current_text.at(cursor_position);
                previous_char = current_text.at(static_cast<qsizetype>(cursor_position) - 1);
            }
            if (cursor_position <= current_text.size() && cursor_position != 0 && cursor_position != 1)
            {
                char_before_last = current_text.at(static_cast<qsizetype>(cursor_position) - 2);
            }
            if (chars_space_skip.count(current_char) > 0 && previous_char == ' ')
            {
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&right);
                QPlainTextEdit::keyPressEvent(event);
            }
            else if (previous_char == '-' && char_before_last == ' ')
            {
                QPlainTextEdit::keyPressEvent(&backspace);
                QPlainTextEdit::keyPressEvent(&enDash);
                QPlainTextEdit::keyPressEvent(event);
            }
            else
            {
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else
        {
            QPlainTextEdit::keyPressEvent(event);
        }
        cursor.endEditBlock();
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
    event->accept();
}

void TextEditor::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
    {
        if (event->angleDelta().y() > 0)
        {
            askFontSliderZoom(true);
        }
        else
        {
            askFontSliderZoom(false);
        }
    }
    else
    {
        QPlainTextEdit::wheelEvent(event);
    }
    event->accept();
}

void TextEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor;
        if (hasLineHighlight == true)
        {
            lineColor = QColor(255, 255, 255, 30);
        }
        else
        {
            lineColor = QColor(0, 0, 0, 0);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void TextEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect()))
    {
        updateLineNumberAreaWidth(0);
    }
}

void TextEditor::scrollUpClicked()
{
    verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
}

void TextEditor::scrollDownClicked()
{
    verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
}

void TextEditor::scrollPreviousClicked()
{
    if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->minimum())
    {
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMinimum);
        return;
    }
    askNavPrevious();
}

void TextEditor::scrollNextClicked()
{
    if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->maximum())
    {
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
        return;
    }
    askNavNext();
}

// texteditor.cpp, fernanda