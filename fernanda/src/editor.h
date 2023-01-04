// editor.h, Fernanda

#pragma once

#include "keyfilter.h"
#include "path.h"
#include "uni.h"

#include <QAbstractSlider>
#include <QColor>
#include <QContextMenuEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLatin1Char>
#include <QObject>
#include <QPainter>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRect>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShortcut>
#include <QSize>
#include <QTextBlock>
#include <QTextFormat>
#include <QTextEdit>
#include <QTimer>
#include <QWheelEvent>
#include <QWidget>

class TextEditor : public QPlainTextEdit
{
    using FsPath = std::filesystem::path;

    Q_OBJECT

public:
    TextEditor(QWidget* parent = nullptr);

    enum class Action {
        None = 0,
        AcceptNew
    };
    enum class Overlay {
        Hide,
        Show
    };
    enum class Scroll {
        Next,
        Previous
    };
    enum class Zoom {
        In,
        Out
    };

    QString cursorColorHex;
    QString cursorUnderColorHex;

    const QStringList devGetCursorPositions();
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    Action handleKeySwap(QString oldKey, QString newKey);
    void handleTextSwap(QString key, QString text);
    int selectedLineCount();
    void scrollNavClicked(Scroll direction);
    void handleFont(FsPath fontPath, int size);

public slots:
    void toggleLineHighlight(bool checked);
    void toggleKeyfilter(bool checked);
    void toggleLineNumberArea(bool checked);
    void toggleScrolls(bool checked);
    void toggleExtraScrolls(bool checked);
    void toggleBlockCursor(bool checked);
    void toggleCursorBlink(bool checked);
    void close(bool isFinal);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    Keyfilter* keyfilter = new Keyfilter;
    QWidget* lineNumberArea;
    QPushButton* scrollUp = new QPushButton(this);
    QPushButton* scrollPrevious = new QPushButton(this);
    QPushButton* scrollNext = new QPushButton(this);
    QPushButton* scrollDown = new QPushButton(this);
    QTimer* cursorBlink = new QTimer(this);

    struct CursorPositions {
        QString key;
        int position;
        int anchor;
        bool operator==(const CursorPositions&) const = default;
        bool operator!=(const CursorPositions&) const = default;
    };

    QVector<CursorPositions> cursorPositions;
    bool hasLineHighlight = true;
    bool hasKeyfilter = true;
    bool hasCursorBlink = true;
    bool hasBlockCursor = true;
    bool cursorVisible = true;

    void keyPresses(QVector<QKeyEvent*> events);
    const QChar currentChar();
    const Keyfilter::ProximalChars proximalChars();
    bool shortcutFilter(QKeyEvent* event);
    void quoteWrap(QKeyEvent* event);
    void connections();
    const QRect reshapeCursor(QChar currentChar);
    const QColor recolorCursor(bool under = false);
    const QColor highlight();
    void storeCursors(QString key);
    void recallCursors(QString key);
    void recallUndoStacks(QString key); // WIP

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);

signals:
    bool askHasProject();
    void askFontSliderZoom(Zoom direction);
    void askNavPrevious();
    void askNavNext();
    void startBlinker();
    void askOverlay(Overlay state);
    bool askHasOverlay();
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor* editor) : QWidget(editor), textEditor(editor) {}

    QSize sizeHint() const override
    {
        return QSize(textEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        textEditor->lineNumberAreaPaintEvent(event);
    }

private:
    TextEditor* textEditor;
};

// editor.h, Fernanda
