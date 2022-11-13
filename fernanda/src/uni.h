// uni.h, fernanda

#pragma once

#include <QString>

namespace Uni
{
	struct {
		QString balloon = "\U0001F388";
		QString file = "\U0001F4C4";
		QString folderClosed = "\U0001F4C1";
		QString folderOpen = "\U0001F4C2";
		QString pushpin = "\U0001F4CC";
		QString triangleDown = "\U000025BC";
		QString triangleUp = "\U000025B2";
		QString unknown = "\U00002754";
	} ico;

	struct {
		QString forbidden = "(<|>|:|\\/|\\\\|\\||\\?|\\*|\")";
		QString themeSheetLine = "(@.*\\n?)";
		QString themeSheetValue = "(\\s=.*;)";
		QString themeSheetVariable = "(@.*=\\s)";
		QString themeSheetCursor = "(@cursorColor; = )(.*)(;$)";
		QString splitCount = "(\\s|\\n|\\r)+";
	} regex;
}

// uni.h, fernanda