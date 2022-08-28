#pragma once

#include "tuple"
#include "unordered_set"

#include <QColor>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLatin1Char>
#include <QList>
#include <QModelIndex>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRect>
#include <QScrollBar>
#include <QSize>
#include <QString>
#include <QTextBlock>
#include <QTextEdit>
#include <QWheelEvent>

using namespace std;

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget* parent = nullptr);
    ~TextEditor() = default;

    QList<tuple<QString, int, int>> cursorPositions;
    bool hasKeyFilters = true;

    QPushButton* scrollPrevious = new QPushButton(this);
    QPushButton* scrollNext = new QPushButton(this);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    void rememberCursorPositions(QString path);
    void restoreCursorPositions(QString path);
    void lineNumberAreaFont(int size, QString fontName);
    void setCursorColor(QString hex);

public slots:
    void toggleLineHight(bool checked);
    void toggleLineNumberArea(bool checked);
    void setTabStop(int pixels);
    void setWrapMode(QTextOption::WrapMode mode);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* lineNumberArea;
    QString cursorColorHex;
    bool hasLineHighlight = true;

    QPushButton* scrollUp = new QPushButton(this);
    QPushButton* scrollDown = new QPushButton(this);

    QKeyEvent backspace{ QKeyEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier };
    QKeyEvent braceRight{ QKeyEvent::KeyPress, Qt::Key_BraceRight, Qt::NoModifier, "}" };
    QKeyEvent bracketRight{ QKeyEvent::KeyPress, Qt::Key_BracketRight, Qt::NoModifier, "]" };
    QKeyEvent deleteKey{ QKeyEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier };
    QKeyEvent emDash{ QKeyEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, "\U00002014" };
    QKeyEvent enDash{ QKeyEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, "\U00002013" };
    QKeyEvent left{ QKeyEvent::KeyPress, Qt::Key_Left, Qt::NoModifier };
    QKeyEvent parenRight{ QKeyEvent::KeyPress, Qt::Key_ParenRight, Qt::NoModifier, ")" };
    QKeyEvent quote{ QKeyEvent::KeyPress, Qt::Key_QuoteDbl, Qt::NoModifier, QString('"') };
    QKeyEvent right{ QKeyEvent::KeyPress, Qt::Key_Right, Qt::NoModifier };
    QKeyEvent space{ QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " " };

    void nameObjects();
    void layoutObjects();
    void makeConnections();
    void paintEvent(QPaintEvent* event);
    const QColor cursorColor();
    void keyPressEvent(QKeyEvent* event);
    void wheelEvent(QWheelEvent* event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void scrollUpClicked();
    void scrollDownClicked();
    void scrollPreviousClicked();
    void scrollNextClicked();

signals:
    void askNavPrevious();
    void askNavNext();
    void askFontSliderZoom(bool zoomDirection);
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor* editor) : QWidget(editor), textEditor(editor)
    {}
    ~LineNumberArea() = default;

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

// texteditor.h, fernanda