// keyfilter.h, Fernanda

#pragma once

#include "unordered_set"

#include <QChar>
#include <QKeyEvent>
#include <QTextCursor>
#include <QVector>

class Keyfilter
{

public:
    struct ProximalChars {
        QChar current;
        QChar previous;
        QChar beforeLast;
    };

    QVector<QKeyEvent*> filter(QKeyEvent* event, ProximalChars chars);

private:
    QKeyEvent backspace{ QKeyEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier };
    QKeyEvent braceRight{ QKeyEvent::KeyPress, Qt::Key_BraceRight, Qt::NoModifier, "}" };
    QKeyEvent bracketRight{ QKeyEvent::KeyPress, Qt::Key_BracketRight, Qt::NoModifier, "]" };
    QKeyEvent emDash{ QKeyEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, "\U00002014" };
    QKeyEvent enDash{ QKeyEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier, "\U00002013" };
    QKeyEvent left{ QKeyEvent::KeyPress, Qt::Key_Left, Qt::NoModifier };
    QKeyEvent parenRight{ QKeyEvent::KeyPress, Qt::Key_ParenRight, Qt::NoModifier, ")" };
    QKeyEvent quote{ QKeyEvent::KeyPress, Qt::Key_QuoteDbl, Qt::NoModifier, QString('"') };
    QKeyEvent right{ QKeyEvent::KeyPress, Qt::Key_Right, Qt::NoModifier };
    //QKeyEvent delete{ QKeyEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier };
    //QKeyEvent guillemetRight{ QKeyEvent::KeyPress, Qt::Key_guillemotright, Qt::NoModifier, "\U000000BB" };
    //QKeyEvent space{ QKeyEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " " };
    std::unordered_set<QChar> spaceSkips{ '}', ']', ',', '!', ')', '.', '?', '"' };
    std::unordered_set<QChar> commaSkips{ '}', ']', ')', '"' };

    QVector<QKeyEvent*> autoClose(QKeyEvent* event, QKeyEvent* closer);
    QVector<QKeyEvent*> dontDuplicate(QKeyEvent* event, ProximalChars chars, char current);
    QVector<QKeyEvent*> commaSkip(QKeyEvent* event, ProximalChars chars);
    bool checkCurrent(ProximalChars chars, char current);
    bool checkPrevious(ProximalChars chars, char previous);
    bool checkPrevAndBeforeLast(ProximalChars chars, char previous, char beforeLast);
};

// keyfilter.h, Fernanda
