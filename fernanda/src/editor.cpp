// editor.cpp, fernanda

#include "editor.h"

TextEditor::TextEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    setReadOnly(true);
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

void TextEditor::lineNumberAreaFont(int size, QString fontName)
{
    QFont font(fontName);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setHintingPreference(QFont::HintingPreference::PreferNoHinting);
    font.setPointSize(size);
    lineNumberArea->setFont(font);
}

bool TextEditor::handleKeySwap(QString oldKey, QString newKey)
{
    if (isReadOnly())
        setReadOnly(false);
    if (oldKey == newKey)
    {
        setFocus();
        return false;
    }
    if (oldKey != nullptr)
        storeCursors(oldKey);
    return true;
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

void TextEditor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    auto c_r = contentsRect();
    lineNumberArea->setGeometry(QRect(c_r.left(), c_r.top(), lineNumberAreaWidth(), c_r.height()));
}

void TextEditor::paintEvent(QPaintEvent* event)
{
    QPlainTextEdit::paintEvent(event);
    const QRect rect = cursorRect(textCursor());
    QPainter painter(viewport());
    painter.fillRect(rect, cursorColor());
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
    if (!hasKeyfilter)
    {
        QPlainTextEdit::keyPressEvent(event);
        return;
    }
    QTextCursor cursor = textCursor();
    int cur_pos = cursor.position();
    QString text = toPlainText();
    auto chars = Keyfilter::ProximalChars{};
    if (cur_pos < text.size())
        chars.current = text.at(cur_pos);
    if (cur_pos > 0)
        chars.previous = text.at(static_cast<qsizetype>(cur_pos) - 1);
    if (cur_pos > 1)
        chars.beforeLast = text.at(static_cast<qsizetype>(cur_pos) - 2);
    auto keys = keyfilter->filter(event, chars);
    cursor.beginEditBlock();
    for (auto& key : keys)
        QPlainTextEdit::keyPressEvent(key);
    cursor.endEditBlock();
}

void TextEditor::connections()
{
    connect(scrollNext, &QPushButton::clicked, this, [&]() { scrollNavClicked(Scroll::Next); });
    connect(scrollPrevious, &QPushButton::clicked, this, [&]() { scrollNavClicked(Scroll::Previous); });
    connect(scrollUp, &QPushButton::clicked, this, [&]()
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        });
    connect(scrollDown, &QPushButton::clicked, this, [&]()
        {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        });
    connect(this, &TextEditor::blockCountChanged, this, &TextEditor::updateLineNumberAreaWidth);
    connect(this, &TextEditor::updateRequest, this, &TextEditor::updateLineNumberArea);
    connect(this, &TextEditor::cursorPositionChanged, this, &TextEditor::highlightCurrentLine);
}

const QColor TextEditor::cursorColor()
{
    QColor cursor_color(cursorColorHex);
    cursor_color.setAlpha(180);
    return cursor_color;
}

void TextEditor::storeCursors(QString key)
{
    for (auto& item : cursors_metaDoc)
        if (key == item.key)
            cursors_metaDoc.removeAll(item);
    cursors_metaDoc << MetaDocCursor{
        key,
        QTextCursor(textCursor()).position(),
        QTextCursor(textCursor()).anchor()
    };
}

void TextEditor::recallCursors(QString key)
{
    for (auto& item : cursors_metaDoc)
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
            cursors_metaDoc.removeAll(item);
            break;
        }
}

void TextEditor::recallUndoStacks(QString key)
{
    //
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
    (dy)
        ? lineNumberArea->scroll(0, dy)
        : lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
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

// editor.cpp, fernanda