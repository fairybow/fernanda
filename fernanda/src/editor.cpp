// editor.cpp, fernanda

#include "editor.h"

TextEditor::TextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    //
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    addScrollBarWidget(scrollUp, Qt::AlignTop);
    addScrollBarWidget(scrollPrevious, Qt::AlignTop);
    addScrollBarWidget(scrollNext, Qt::AlignBottom);
    addScrollBarWidget(scrollDown, Qt::AlignBottom);
    scrollUp->setAutoRepeat(true);
    scrollDown->setAutoRepeat(true);
    scrollUp->setText("\U000025B2");
    scrollPrevious->setText("\U000025B2");
    scrollNext->setText("\U000025BC");
    scrollDown->setText("\U000025BC");
    for (auto& button : { scrollUp, scrollPrevious, scrollNext, scrollDown })
        button->setMinimumHeight(30);
    lineNumberArea->setObjectName("lineNumberArea");
    horizontalScrollBar()->setObjectName("hScrollBar");
    verticalScrollBar()->setObjectName("vScrollBar");
    scrollUp->setObjectName("scrollUp");
    scrollPrevious->setObjectName("scrollPrevious");
    scrollNext->setObjectName("scrollNext");
    scrollDown->setObjectName("scrollDown");
    //
    connect(scrollUp, &QPushButton::clicked, this, [&]()
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        });
    connect(scrollDown, &QPushButton::clicked, this, [&]()
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        });
    connect(scrollPrevious, &QPushButton::clicked, this, &TextEditor::scrollPreviousClicked);
    connect(scrollNext, &QPushButton::clicked, this, &TextEditor::scrollNextClicked);
    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void TextEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(255, 255, 255, 15));
    auto block = firstVisibleBlock();
    auto block_number = block.blockNumber();
    auto top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    auto bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            auto number = QString::number(block_number + 1);
            painter.setPen(QColor(255, 255, 255, 30));
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, number);
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

void TextEditor::lineNumberAreaFont(int size, QString fontName)
{
    QFont font(fontName);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    font.setPointSize(size);
    lineNumberArea->setFont(font);
}

void TextEditor::toggleLineHighlight(bool checked)
{
    hasLineHighlight = checked;
    highlightCurrentLine();
}

void TextEditor::toggleLineNumberArea(bool checked)
{
    lineNumberArea->setVisible(checked);
    updateLineNumberAreaWidth(0);
}

void TextEditor::storeCursors(QString relFilePath)
{
    auto cursor_pos = QTextCursor(textCursor()).position();
    auto anchor_pos = QTextCursor(textCursor()).anchor();
    cursorPositions << std::tuple<QString, int, int>(relFilePath, cursor_pos, anchor_pos);
}

void TextEditor::recallCursors(QString relFilePath)
{
    for (auto& tuple : cursorPositions)
        if (relFilePath == std::get<0>(tuple))
        {
            auto cursor(textCursor());
            auto cursor_pos = std::get<1>(tuple);
            auto anchor_pos = std::get<2>(tuple);
            if (cursor_pos == anchor_pos)
                cursor.setPosition(cursor_pos);
            else
            {
                cursor.setPosition(anchor_pos, QTextCursor::MoveAnchor);
                cursor.setPosition(cursor_pos, QTextCursor::KeepAnchor);
            }
            setTextCursor(cursor);
            cursorPositions.removeAll(tuple);
            break;
        }
}

void TextEditor::storeUndoStacks(QString relFilePath)
{
    //
}

void TextEditor::recallUndoStacks(QString relFilePath)
{
    //
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
    if (!hasFocus()) return;
    const QRect rect = cursorRect(textCursor());
    QPainter painter(viewport());
    painter.fillRect(rect, cursorColor());
}

const QColor TextEditor::cursorColor()
{
    QColor cursor_color(cursorColorHex);
    cursor_color.setAlpha(180);
    return cursor_color;
}

void TextEditor::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() == Qt::ControlModifier)
        if (event->angleDelta().y() > 0)
            askFontSliderZoom(true);
        else
            askFontSliderZoom(false);
    else
        QPlainTextEdit::wheelEvent(event);
    event->accept();
}

void TextEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEditor::highlightCurrentLine()
{
    QVector<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(0, 0, 0, 0);
        if (hasLineHighlight)
            lineColor = QColor(255, 255, 255, 30);
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
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TextEditor::scrollPreviousClicked()
{
    auto project = askHasProject();
    if (!project) return;
    if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->minimum())
    {
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMinimum);
        return;
    }
    askNavPrevious();
}

void TextEditor::scrollNextClicked()
{
    auto project = askHasProject();
    if (!project) return;
    if (verticalScrollBar()->sliderPosition() != verticalScrollBar()->maximum())
    {
        verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
        return;
    }
    askNavNext();
}

// editor.cpp, fernanda