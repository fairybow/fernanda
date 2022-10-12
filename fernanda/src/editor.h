// editor.h, fernanda

#pragma once

#include <tuple>

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
#include <QSize>
#include <QString>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextEdit>
#include <QVector>
#include <QWheelEvent>
#include <QWidget>

class TextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEditor(QWidget* parent = nullptr);
    ~TextEditor() = default;
    QString cursorColorHex;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    void lineNumberAreaFont(int size, QString fontName);

public slots:
    void toggleLineHighlight(bool checked);
    void toggleLineNumberArea(bool checked);
    void storeCursors(QString relFilePath);
    void recallCursors(QString relFilePath);
    void storeUndoStacks(QString relFilePath);
    void recallUndoStacks(QString relFilePath);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QWidget* lineNumberArea;
    QPushButton* scrollUp = new QPushButton(this);
    QPushButton* scrollPrevious = new QPushButton(this);
    QPushButton* scrollNext = new QPushButton(this);
    QPushButton* scrollDown = new QPushButton(this);
    QVector<std::tuple<QString, int, int>> cursorPositions;
    bool hasLineHighlight = true;
    void paintEvent(QPaintEvent* event);
    const QColor cursorColor();
    void wheelEvent(QWheelEvent* event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount); // <- this isn't used?
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void scrollPreviousClicked();
    void scrollNextClicked();

signals:
    bool askHasProject();
    void askFontSliderZoom(bool zoomDirection);
    void askNavPrevious();
    void askNavNext();
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(TextEditor* editor) : QWidget(editor), textEditor(editor) {}
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

// editor.h, fernanda