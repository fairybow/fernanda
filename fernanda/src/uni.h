// uni.h, Fernanda

#pragma once

#include "version.h"

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
		NewLine,
		ParagraphSeparator,
		Space,
		Split,
		ThemeSheetCursor,
		ThemeSheetCursorUnder,
		ThemeSheetLine,
		ThemeSheetValue,
		ThemeSheetVariable
	};

	enum class Version {
		Error,
		Latest,
		Old
	};

	inline const QString ico(Ico name)
	{
		QString result;
		switch (name) {
		case Ico::Balloon:
			result = QStringLiteral("\U0001F388");
			break;
		case Ico::File:
			result = QStringLiteral("\U0001F4C4");
			break;
		case Ico::FolderClosed:
			result = QStringLiteral("\U0001F4C1");
			break;
		case Ico::FolderOpen:
			result = QStringLiteral("\U0001F4C2");
			break;
		case Ico::Pushpin:
			result = QStringLiteral("\U0001F4CC");
			break;
		case Ico::QuestionMark:
			result = QStringLiteral("\U00002754");
			break;
		case Ico::TriangleDown:
			result = QStringLiteral("\U000025BC");
			break;
		case Ico::TriangleUp:
			result = QStringLiteral("\U000025B2");
			break;
		}
		return result;
	}

	inline const QRegularExpression regex(Re operation)
	{
		QRegularExpression result;
		switch (operation) {
		case Re::Forbidden:
			result = QRegularExpression(QStringLiteral(R"((<|>|:|\/|\\|\||\?|\*|\"))"));
			break;
		case Re::NewLine:
			result = QRegularExpression(QStringLiteral("(\\n)"));
			break;
		case Re::ParagraphSeparator:
			result = QRegularExpression(QStringLiteral("(\U00002029)"));
			break;
		case Re::Space:
			result = QRegularExpression(QStringLiteral("(\\s)"));
			break;
		case Re::Split:
			result = QRegularExpression(QStringLiteral("(\\s|\\n|\\r|\U00002029|^)+"));
			break;
		case Re::ThemeSheetCursor:
			result = QRegularExpression(QStringLiteral("(@cursorColor; = )(.*)(;)"));
			break;
		case Re::ThemeSheetCursorUnder:
			result = QRegularExpression(QStringLiteral("(@cursorUnderColor; = )(.*)(;)"));
			break;
		case Re::ThemeSheetLine:
			result = QRegularExpression(QStringLiteral("(@.*\\n?)"));
			break;
		case Re::ThemeSheetValue:
			result = QRegularExpression(QStringLiteral("(\\s=.*;)"));
			break;
		case Re::ThemeSheetVariable:
			result = QRegularExpression(QStringLiteral("(@.*=\\s)"));
			break;
		}
		return result;
	}

	inline const QString change(bool isQuit = false)
	{
		QString result;
		auto base = QStringLiteral("You have <b>unsaved changes</b>. Are you sure you want to ");
		(isQuit)
			? result = base + "quit?"
			: result = base + "change stories?";
		return result;
	}

	inline const char* saveAnd(bool isQuit = false)
	{
		if (isQuit) return "  Save and quit  ";
		return "  Save and change  ";
	}

	inline const QString samples()
	{
		return QStringLiteral("A sample font, window theme, and editor theme have been added to your user data folder.<p>You'll need to <b>restart</b> to see custom themes and fonts incorporated.");
	}

	inline const QString menuShortcuts()
	{
		return QStringLiteral("<b>Menu:</b><p>Ctrl + S: Save story<br>Ctrl + Q: Quit");
	}

	inline const QString windowShortcuts()
	{
		return QStringLiteral("<b>Window:</b><p>F11: Cycle editor themes (Amber, Green, Grey)<br>Alt + F10: Cycle fonts<br>Alt + F11: Cycle editor themes (all)<br>Alt + F12: Cycle window themes<br>Alt + Insert: Nav previous<br>Alt + Delete: Nav Next<br>Alt + Minus (-) /<br>Ctrl + Mouse Wheel Down: Decrease font size<br>Alt + Plus (+) /<br>Ctrl + Mouse Wheel Up: Increase font size");
	}

	inline const QString editorShortcuts()
	{
		return QStringLiteral("<b>Editor:</b><p>Ctrl + Y: Redo<br>Ctrl + Z: Undo<br>Ctrl + Shift + C: Wrap selection or block in quotes");
	}

	inline const QString shortcuts()
	{
		return QStringLiteral("<table><td>") + menuShortcuts() + QStringLiteral("</td><td>\n</td><td>") + windowShortcuts() + QStringLiteral("</td><td>\n</td><td>") + editorShortcuts() + QStringLiteral("</td><table>");
	}

	inline const QString repo()
	{
		return QStringLiteral("<a href='https://github.com/fairybow/fernanda'>github.com/fairybow/fernanda</a>");
	}

	inline const QString releases()
	{
		return QStringLiteral("<a href='https://github.com/fairybow/fernanda/releases/'>github.com/fairybow/fernanda/releases</a>");
	}

	inline const QString about()
	{
		return QStringLiteral("<b>Fernanda</b> is a personal project and a work-in-progress.<p><b>Version:</b> ") + VER_FILEVERSION_STR + "<p>" + repo();
	}

	inline const QString version(Version check)
	{
		QString result;
		switch (check) {
		case Version::Error:
			result = QStringLiteral("Unable to verify version.<p><b>Check:</b> ") + releases();
			break;
		case Version::Latest:
			result = QStringLiteral("You have the latest version!");
			break;
		case Version::Old:
			result = QStringLiteral("You do not have the latest version.<p><b>Download:</b> ") + releases();
			break;
		}
		return result;
	}
}

// uni.h, Fernanda
