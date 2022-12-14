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
			result = QRegularExpression("(@cursorColor; = )(.*)(;$)");
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

	inline const QString hotkeys()
	{
		return "<b>Alt + F10:</b> Cycle fonts<br><b>F11:</b> Cycle editor themes (Amber, Green, Grey)<br><b>Alt + F11:</b> Cycle editor themes (all)<br><b>Alt + F12:</b> Cycle window themes<br><b>Alt + Insert:</b> Nav previous<br><b>Alt + Delete:</b> Nav Next<br><b>Alt + - (Ctrl + Mouse Wheel Down):</b> Decrease font size<br><b>Alt + + (Ctrl + Mouse Wheel Up):</b> Increase font size";
	}
}

// uni.h, fernanda