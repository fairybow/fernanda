#include "pane_delegate.h"

void PaneDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFont font;
    QFont italics;
    italics.setItalic(true);
    painter->setFont(font);
    auto option_size = QSize(option.rect.height(), option.rect.height());
    auto option_rect = option.rect;
    auto text_rect = QRect((option_rect.left()), (option_rect.top() + 2), option_rect.width(), option_rect.height());
    //auto text_rect = QRect((option_rect.left() + 15), (option_rect.top() + 2), option_rect.width(), option_rect.height());
    auto highlight = QRect(5, option_rect.top(), (option_rect.width() + 50), option_rect.height());
    /*auto folder_icon_rect = QRect((option_rect.left() - option_size.width() + 18), (option_rect.top() + 2), option_size.width(), option_size.height());
    auto file_icon_rect = QRect((option_rect.left() - option_size.width() + 15), (option_rect.top()), option_size.width(), option_size.height());
    QPixmap folder_icon(":/icons/folder.png");
    QPixmap file_icon(":/icons/file.png");
    QFileInfo check_index(dirModel->fileInfo(index).absoluteFilePath());
    if (check_index.isDir())
    {
        QDir _index(check_index.absoluteFilePath());
        painter->drawPixmap(folder_icon_rect, folder_icon, folder_icon.rect());
    }
    else
    {
        painter->drawPixmap(file_icon_rect, file_icon, file_icon.rect());
    }*/
    painter->drawText(text_rect, index.data().toString());
    for (auto& entry : dirtyIndexes)
        if (index == entry)
        {
            painter->eraseRect(text_rect);
            painter->setFont(italics);
            painter->drawText(text_rect, index.data().toString() + "*");
        }
    if (option.state & QStyle::State_MouseOver || option.state & QStyle::State_Selected)
        painter->fillRect(highlight, QColor(0, 0, 0, 33));
}

// pane_delegate.cpp, fernanda