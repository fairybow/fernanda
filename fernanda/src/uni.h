// uni.h, Fernanda

#pragma once

#include "version.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>

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

	const QString operator%(QString lhs, const char* rhs);
	const QString operator%(QString lhs, QString rhs);
	const QString operator/(QString lhs, const char* rhs);
	const QString operator/(QString lhs, QString rhs);
	const QString ico(Ico name);
	const QRegularExpression regex(Re operation);
	const QString heading(const char* text);
	const QString bold(const char* text);
	const QString pad(const char* text);
	const QString table(QStringList columns);
	const QString link(const char* url, const char* name);
	const QString change(bool isQuit = false);
	const QString saveAndButtons(bool isQuit = false);
	const QString openUdButton();
	const QString samples();
	const QString menuShortcuts();
	const QString windowShortcuts();
	const QString editorShortcuts();
	const QString shortcuts();
	const QString repo();
	const QString releases();
	const QString about();
	const QString version(Version check, QString latestVersion);
}

// uni.h, Fernanda
