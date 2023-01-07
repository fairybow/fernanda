# Dependencies

## Tree

main
	- fernanda.h
		- colorbar.h
			- io.h
				- path.h
		- editor.h
			- keyfilter.h
			- path.h
			- uni.h
				- version.h
		- indicator.h
			- uni.h
				- version.h
		- pane.h
			- delegate.h
				- index.h
				- uni.h
					- version.h
			- io.h
				- path.h
		- popup.h
			- uni.h
				- version.h
		- res.h
			- path.h
			- uni.h
				- version.h
		- splitter.h
			- userdata.h
				- path.h
		- story.h
			- archiver.h
				- io.h
					- path.h
				- userdata.h
					- path.h
			- dom.h
				- io.h
					- path.h
			- sample.h
				- io.h
					- path.h
			- uni.h
				- version.h
	- startcop.h

## List

### archiver.h
```
#include "bit7z/include/bit7z.hpp"
#include "bit7z/include/bitarchiveeditor.hpp"

#include "io.h"
#include "userdata.h"

#include <map>
#include <vector>

#include <QTemporaryDir>
```

### colorbar.h
```
#include "io.h"

#include <QProgressBar>
#include <QTimeLine>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
```

### delegate.h
```
#include "index.h"
#include "uni.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QWidget>
```

### dom.h
```
#include "io.h"

#include <QDomDocument>
#include <QDomElement>
#include <QUuid>
#include <QVector>
```

### editor.h
```
#include "keyfilter.h"
#include "path.h"
#include "uni.h"

#include <QAbstractSlider>
#include <QColor>
#include <QContextMenuEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLatin1Char>
#include <QObject>
#include <QPainter>
#include <QPaintEvent>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRect>
#include <QResizeEvent>
#include <QScrollBar>
#include <QShortcut>
#include <QSize>
#include <QTextBlock>
#include <QTextFormat>
#include <QTextEdit>
#include <QTimer>
#include <QWheelEvent>
#include <QWidget>
```

### fernanda.h
```
#include "colorbar.h"
#include "editor.h"
#include "indicator.h"
#include "pane.h"
#include "popup.h"
#include "res.h"
#include "splitter.h"
#include "story.h"

#include <QAbstractButton>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGraphicsBlurEffect>
#include <QJsonDocument>
#include <QMainWindow>
#include <QMap>
#include <QMenuBar>
#include <QMoveEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QShowEvent>
#include <QShortcut>
#include <QSizePolicy>
#include <QSlider>
#include <QStatusBar>
#include <QStackedLayout>
#include <QTextOption>
#include <QTextTable>
#include <QUrl>
#include <QWidgetAction>
```

### index.h
```
#include <type_traits>

#include <QModelIndex>
#include <QString>
#include <Qt>
#include <QVariant>
```

### indicator.h
```
#include "uni.h"

#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
```

### io.h
```
#include "path.h"

#include <optional>
#include <utility>

#include <QFile>
#include <QIODevice>
#include <QTextStream>
```

### keyfilter.h
```
#include "unordered_set"

#include <QChar>
#include <QKeyEvent>
#include <QTextCursor>
#include <QVector>
```

### pane.h
```
#include "delegate.h"
#include "io.h"

#include <QAbstractItemView>
#include <QAction>
#include <QContextMenuEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPoint>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
```

### path.h
```
#include <filesystem>
#include <string>
#include <type_traits>

#include <qsystemdetection.h>

#include <QDir>
#include <QFileInfo>
#include <QString>
```

### popup.h
```
#include "uni.h"

#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QWidget>
```

### res.h
```
#include "path.h"
#include "uni.h"

#include <algorithm>

#include <QDirIterator>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QStringList>
#include <QVector>
```

### sample.h
```
#include "io.h"

#include <QDirIterator>
#include <QStringList>
#include <QVector>
```

### splitter.h
```
#include "userdata.h"

#include <QByteArray>
#include <QSplitter>
#include <QVector>
#include <QWidget>
```

### startcop.h
```
#include <string>

#include <qsystemdetection.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QObject>
#include <QSharedMemory>
#include <QString>
#include <QStringList>
#include <QSystemSemaphore>

#ifdef Q_OS_WINDOWS

#include <windows.h>
#include <winuser.h>

#else

//

#endif
```

### story.h
```
#include "archiver.h"
#include "dom.h"
#include "sample.h"
#include "uni.h"

#include <QStandardItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
```

### uni.h
```
#include "version.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>
```

### userdata.h
```
#include "path.h"

#include <filesystem>
#include <string>
#include <time.h>

#include <qsystemdetection.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRect>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QVariant>
```

### version.h
```
```