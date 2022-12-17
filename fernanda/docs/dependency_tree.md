Out of date!

# Dependencies

## Tree

- fernanda.h
	- colorbar.h
		- io.h
		- path.h
	- editor.h
		- keyfilter.h
		- uni.h
	- indicator.h
		- uni.h
	- pane.h
		- delegate.h
			- index.h
			- uni.h
		- io.h
			- path.h
	- res.h
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

## List

### archiver.h
```
#include "bit7z/include/bit7z.hpp"

#include "userdata.h"

#include <map>
#include <type_traits>
```

### colorbar.h
```
#include "io.h"

#include <QProgressBar>
#include <QString>
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
#include <QVector>
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
#include "uni.h"

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
#include <QShortcut>
#include <QSize>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextEdit>
#include <QWheelEvent>
#include <QWidget>
```

### fernanda.h
```
#include "colorbar.h"
#include "editor.h"
#include "indicator.h"
#include "pane.h"
#include "project.h"
#include "res.h"
#include "splitter.h"

#include <tuple>

#include <QAbstractButton>
#include <QActionGroup>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMoveEvent>
#include <QShowEvent>
#include <QSizePolicy>
#include <QSlider>
#include <QStatusBar>
#include <QStackedLayout>
#include <QTextOption>
#include <QWidgetAction>
```

### index.h
```
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
#include <QString>
#include <QStringList>
#include <QWidget>
```

### io.h
```
#include "path.h"

#include <optional>

#include <QFile>
#include <QIODevice>
#include <QTextStream>
```

### keyfilter.h
```
#include <QChar>
#include <QKeyEvent>
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
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTreeView>
```

### path.h
```
#include <filesystem>

#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
```

### res.h
```
#include <algorithm>
#include <filesystem>
#include <string>

#include "uni.h"

#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QString>
#include <QStringList>
#include <QVector>
```

### sample.h
```
#include "io.h"

#include <filesystem>
#include <string>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QString>
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

### story.h
```
#include "archiver.h"
#include "dom.h"
#include "sample.h"

#include <QByteArray>
#include <QStandardItem>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
```

### uni.h
```
#include <QRegularExpression>
#include <QString>
```

### userdata.h
```
#include <shlobj_core.h>

#include <filesystem>
#include <string>
#include <time.h>

#include "path.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QRect>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QVariant>
```

### version.h
```
```