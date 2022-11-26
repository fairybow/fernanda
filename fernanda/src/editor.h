// editor.h, fernanda

#pragma once

#include "keyfilter.h"
#include "uni.h"

#include <QAbstractSlider>
#include <QColor>
#include <QFont>
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
#include <QTextCursor>
#include <QTextFormat>
#include <QTextEdit>
#include <QWheelEvent>
#include <QWidget>

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget* parent = nullptr);

    enum class Zoom {
        In,
        Out
    };

    QString cursorColorHex;

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    void lineNumberAreaFont(int size, QString fontName);
    bool handleKeySwap(QString oldKey, QString newKey);
    void handleTextSwap(QString key, QString text);
    int selectedLineCount();

public slots:
    void toggleLineHighlight(bool checked);
    void toggleKeyfilter(bool checked);
    void toggleLineNumberArea(bool checked);
    void toggleScrolls(bool checked);
    void toggleExtraScrolls(bool checked);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Keyfilter* keyfilter = new Keyfilter;
    QWidget* lineNumberArea;
    QPushButton* scrollUp = new QPushButton(this);
    QPushButton* scrollPrevious = new QPushButton(this);
    QPushButton* scrollNext = new QPushButton(this);
    QPushButton* scrollDown = new QPushButton(this);

    struct MetaDocCursor {
        QString key;
        int position;
        int anchor;
        bool operator==(const MetaDocCursor&) const = default;
        bool operator!=(const MetaDocCursor&) const = default;
    };

    QVector<MetaDocCursor> cursors_metaDoc;

    bool hasLineHighlight = true;
    bool hasKeyfilter = true;

    void connections();
    const QColor cursorColor();
    void storeCursors(QString key);
    void recallCursors(QString key);
    void recallUndoStacks(QString key); // WIP

private slots:
    void updateLineNumberAreaWidth(int newBlockCount); // <- this arg isn't used?
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void scrollPreviousClicked();
    void scrollNextClicked();

signals:
    bool askHasProject();
    void askFontSliderZoom(Zoom direction);
    void askNavPrevious();
    void askNavNext();
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

// editor.h, fernanda