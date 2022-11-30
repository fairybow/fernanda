// keyfilter.h, fernanda

#pragma once

#include "unordered_set"

#include <QChar>
#include <QKeyEvent>
#include <QVector>

class Keyfilter
{

public:
    struct ProximalChars {
        QChar current;
        QChar previous;
        QChar beforeLast;
    };

    QVector<QKeyEvent*> filter(QKeyEvent* event, ProximalChars chars)
    {
        QVector<QKeyEvent*> result;
        switch (event->key()) {
        case Qt::Key_BraceLeft:
            result << autoClose(event, &braceRight);
            break;
        case Qt::Key_BraceRight:
            result << dontDuplicate(event, chars, '}');
            break;
        case Qt::Key_BracketLeft:
            result << autoClose(event, &bracketRight);
            break;
        case Qt::Key_BracketRight:
            result << dontDuplicate(event, chars, ']');
            break;
        case Qt::Key_Comma:
            result << commaSkip(event, chars);
            break;
        case Qt::Key_Exclam:
            result << commaSkip(event, chars);
            break;
        case Qt::Key_Minus:
            (checkPrevious(chars, '-'))
                ? result << &backspace << &emDash
                : result << event;
            break;
        case Qt::Key_ParenLeft:
            result << autoClose(event, &parenRight);
            break;
        case Qt::Key_ParenRight:
            result << dontDuplicate(event, chars, ')');
            break;
        case Qt::Key_Period:
            result << commaSkip(event, chars);
            break;
        case Qt::Key_Question:
            result << commaSkip(event, chars);
            break;
        case Qt::Key_QuoteDbl:
            (checkCurrent(chars, '"'))
                ? result << &right
                : result << autoClose(event, event);
            break;
        case Qt::Key_Space:
        {
            if (!chars.current.isNull() && !chars.previous.isNull() && spaceSkips.count(chars.current) > 0 && chars.previous == ' ')
                result << &backspace << &right << event;
            else if (checkPrevAndBeforeLast(chars, '-', ' '))
                result << &backspace << &enDash << event;
            else
                result << event;
        }
            break;
        default:
            (checkPrevAndBeforeLast(chars, ' ', ' '))
                ? result << &backspace << event
                : result << event;
        }
        return result;
    }

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

    QVector<QKeyEvent*> autoClose(QKeyEvent* event, QKeyEvent* closer)
    {
        return QVector<QKeyEvent*>{ event, closer, & left };
    }

    QVector<QKeyEvent*> dontDuplicate(QKeyEvent* event, ProximalChars chars, char current)
    {
        QVector<QKeyEvent*> result;
        (checkCurrent(chars, current))
            ? result << &right
            : result << event;
        return result;
    }

    QVector<QKeyEvent*> commaSkip(QKeyEvent* event, ProximalChars chars)
    {
        QVector<QKeyEvent*> result;
        if (!chars.current.isNull() && commaSkips.count(chars.current) > 0 && chars.previous == ',')
            result << &backspace << event << &right;
        else
            result << event;
        return result;
    }

    bool checkCurrent(ProximalChars chars, char current)
    {
        if (!chars.current.isNull() && chars.current == current)
            return true;
        return false;
    }

    bool checkPrevious(ProximalChars chars, char previous)
    {
        if (!chars.previous.isNull() && chars.previous == previous)
            return true;
        return false;
    }

    bool checkPrevAndBeforeLast(ProximalChars chars, char previous, char beforeLast)
    {
        if (!chars.previous.isNull() && !chars.beforeLast.isNull() && chars.previous == previous && chars.beforeLast == beforeLast)
            return true;
        return false;
    }
};

// keyfilter.h, fernanda