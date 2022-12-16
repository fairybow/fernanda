// uni.h, fernanda

#pragma once

#include <QRegularExpression>
#include <QString>

namespace Uni
{
	enum class Ico {
		Balloon,
		File,
		FolderClosed,
		FolderOpen,
		Pushpin,
		QuestionMark,
		TriangleDown,
		TriangleUp
	};

	enum class Re {
		Forbidden,
		ParagraphSeparator,
		Split,
		ThemeSheetCursor,
		ThemeSheetCursorUnder,
		ThemeSheetLine,
		ThemeSheetValue,
		ThemeSheetVariable
	};

	inline const QString ico(Ico name)
	{
		QString result;
		switch (name) {
		case Ico::Balloon:
			result = "\U0001F388";
			break;
		case Ico::File:
			result = "\U0001F4C4";
			break;
		case Ico::FolderClosed:
			result = "\U0001F4C1";
			break;
		case Ico::FolderOpen:
			result = "\U0001F4C2";
			break;
		case Ico::Pushpin:
			result = "\U0001F4CC";
			break;
		case Ico::QuestionMark:
			result = "\U00002754";
			break;
		case Ico::TriangleDown:
			result = "\U000025BC";
			break;
		case Ico::TriangleUp:
			result = "\U000025B2";
			break;
		}
		return result;
	}

	inline const QRegularExpression regex(Re operation)
	{
		QRegularExpression result;
		switch (operation) {
		case Re::Forbidden:
			result = QRegularExpression(R"((<|>|:|\/|\\|\||\?|\*|\"))");
			break;
		case Re::ParagraphSeparator:
			result = QRegularExpression("\U00002029");
			break;
		case Re::Split:
			result = QRegularExpression("(\\s|\\n|\\r|\U00002029|^)+");
			break;
		case Re::ThemeSheetCursor:
			result = QRegularExpression("(@cursorColor; = )(.*)(;)");
			break;
		case Re::ThemeSheetCursorUnder:
			result = QRegularExpression("(@cursorUnderColor; = )(.*)(;)");
			break;
		case Re::ThemeSheetLine:
			result = QRegularExpression("(@.*\\n?)");
			break;
		case Re::ThemeSheetValue:
			result = QRegularExpression("(\\s=.*;)");
			break;
		case Re::ThemeSheetVariable:
			result = QRegularExpression("(@.*=\\s)");
			break;
		}
		return result;
	}

	inline const QString close()
	{
		return "You have <b>unsaved changes</b>. Are you sure you want to quit?";
	}

	inline const QString samples()
	{
		return "You'll need to <b>restart</b> to see custom themes and fonts incorporated.";
	}

	inline const QString menuShortcuts()
	{
		return "<b>Menu:</b><p>Ctrl + S: Save story<br>Ctrl + Q: Quit";
	}

	inline const QString windowShortcuts()
	{
		return "<b>Window:</b><p>F11: Cycle editor themes (Amber, Green, Grey)<br>Alt + F10: Cycle fonts<br>Alt + F11: Cycle editor themes (all)<br>Alt + F12: Cycle window themes<br>Alt + Insert: Nav previous<br>Alt + Delete: Nav Next<br>Alt + Minus (-) /<br>Ctrl + Mouse Wheel Down: Decrease font size<br>Alt + Plus (+) /<br>Ctrl + Mouse Wheel Up: Increase font size";
	}

	inline const QString editorShortcuts()
	{
		return "<b>Editor:</b><p>Ctrl + Y: Redo<br>Ctrl + Z: Undo<br>Ctrl + Shift + C: Wrap selection or block in quotes";
	}

	inline const QString shortcuts()
	{
		return "<table><td>" + menuShortcuts() + "</td><td>\n</td><td>" + windowShortcuts() + "</td><td>\n</td><td>" + editorShortcuts() + "</td><table>";
	}

	inline const QString about()
	{
		return "<b>Fernanda</b> is a personal project and a work-in-progress.<p>Version: hella beta<p><a href='https://github.com/fairybow/fernanda'>github.com/fairybow/fernanda</a>";
	}
}

// uni.h, fernanda