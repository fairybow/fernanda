// uni.cpp, Fernanda

#include "uni.h"

const QString Uni::operator%(QString lhs, const char* rhs)
{
	return lhs + QStringLiteral("<p>") + rhs;
}

const QString Uni::operator%(QString lhs, QString rhs)
{
	return lhs + QStringLiteral("<p>") + rhs;
}

const QString Uni::operator/(QString lhs, const char* rhs)
{
	return lhs + QStringLiteral("<br>") + rhs;
}

const QString Uni::operator/(QString lhs, QString rhs)
{
	return lhs + QStringLiteral("<br>") + rhs;
}

const QString Uni::ico(Ico name)
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

const QRegularExpression Uni::regex(Re operation)
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
	case Re::UrlBeginning:
		result = QRegularExpression(QStringLiteral("(https:\\/\\/|www.)"));
		break;
	}
	return result;
}

const QString Uni::multiplyThese(QString character, int defaultArg)
{
	if (defaultArg < 1)
		defaultArg = 1;
	QString result;
	for (auto i = 0; i < defaultArg; ++i)
		result.append(character);
	return result;
}

const QString Uni::spaces(int spaces)
{
	return multiplyThese(" ", 3);
}

const QString Uni::newLines(int lines)
{
	return multiplyThese("\n", 2);
}

const QString Uni::heading(const char* text)
{
	return QStringLiteral("<h3><b>") + text + QStringLiteral("</b></h3>");
}

const QString Uni::bold(const char* text)
{
	return QStringLiteral("<b>") + text + QStringLiteral("</b>");
}

const QString Uni::pad(const char* text)
{
	QString padding = spaces();
	return padding + text + padding;
}

const QString Uni::table(QStringList columns)
{
	QString result = QStringLiteral("<table><td>");
	for (auto& column : columns)
	{
		if (column != columns.last())
			result.append(column).append(QStringLiteral("</td><td>\n</td><td>"));
		else
			result.append(column).append(QStringLiteral("</td><table>"));
	}
	return result;
}

const QString Uni::link(const char* url, QString displayName)
{
	if (displayName.isEmpty())
		displayName = QString(url).replace(regex(Re::UrlBeginning), nullptr);
	return QStringLiteral("<a href='") + url + QStringLiteral("'>") + displayName + QStringLiteral("</a>");
}

const QString Uni::change(bool isQuit)
{
	QString result;
	auto base = QStringLiteral("You have ") + bold("unsaved changes") + QStringLiteral(". Are you sure you want to ");
	(isQuit)
		? result = base + QStringLiteral("quit?")
		: result = base + QStringLiteral("change stories?");
	return result;
}

const QString Uni::saveAndButtons(bool isQuit)
{
	if (isQuit)
		return pad("Save and quit");
	return pad("Save and change");
}

const QString Uni::openUdButton()
{
	return pad("Open the user data folder");
}

const QString Uni::samples()
{
	return
	{
		QStringLiteral("A sample font, window theme, and editor theme have been added to your user data folder.") %
		QStringLiteral("You'll need to ") + bold("restart") + QStringLiteral(" to see custom themes and fonts incorporated.")
	};
}

const QString Uni::menuShortcuts()
{
	return
	{
		bold("Menu:") %
		QStringLiteral("Ctrl + S: Save story") /
		QStringLiteral("Ctrl + Q: Quit")
	};
}

const QString Uni::windowShortcuts()
{
	return
	{
		bold("Window:") %
		QStringLiteral("F11: Cycle editor themes (Amber, Green, Grey)") /
		QStringLiteral("Alt + F10: Cycle fonts") /
		QStringLiteral("Alt + F11: Cycle editor themes (all)") /
		QStringLiteral("Alt + F12: Cycle window themes") /
		QStringLiteral("Alt + Insert: Nav previous") /
		QStringLiteral("Alt + Delete: Nav next") /
		QStringLiteral("Alt + Minus (-) /") /
		QStringLiteral("Ctrl + Mouse Wheel Down: Decrease font size") /
		QStringLiteral("Alt + Plus (+) /") /
		QStringLiteral("Ctrl + Mouse Wheel Up: Increase font size")
	};
}

inline const QString Uni::editorShortcuts()
{
	return
	{
		bold("Editor:") %
		QStringLiteral("Ctrl + Y: Redo") /
		QStringLiteral("Ctrl + Z: Undo") /
		QStringLiteral("Ctrl + Shift + C: Wrap selection or block in quotes")
	};
}

const QString Uni::shortcuts()
{
	return heading("Shortcuts") % table({ menuShortcuts(), windowShortcuts(), editorShortcuts() });
}

const QString Uni::repo()
{
	return link("https://github.com/fairybow/fernanda");
}

const QString Uni::releases()
{
	return link("https://github.com/fairybow/fernanda/releases");
}

const QString Uni::about()
{
	return
	{
		heading("About Fernanda") %
		QStringLiteral("Fernanda is a plain text editor for drafting long-form fiction. (At least, that's the plan.)") %
		QStringLiteral("It's a personal project and a work-in-progress.") %
		heading("Version") %
		VER_FILEVERSION_STR %
		QStringLiteral("See ") + repo() + QStringLiteral(" for more information.")
	};
}

const QString Uni::version(Version check, QString latestVersion)
{
	QString base =
	{
		heading("Version") %
		bold("Current version:") /
		VER_FILEVERSION_STR
	};
	QString message;
	switch (check) {
	case Version::Error:
		message =
		{
			QStringLiteral("Unable to verify version.") %
			bold("Check:") /
			releases()
		};
		break;
	case Version::Latest:
		message = QStringLiteral("You have the latest version.");
		break;
	case Version::Old:
		message =
		{
			bold("New version:") /
			latestVersion %
			QStringLiteral("You do not have the latest version.") %
			bold("Download:") /
			releases()
		};
		break;
	}
	return
	{
		base %
		message
	};
}

// uni.cpp, Fernanda
