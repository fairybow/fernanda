#include "io.h"

const QString readFile(QString path)
{
    QString text;
    QFile file(path);
    if (file.open(QFile::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        text = in.readAll();
        file.close();
    }
    return text;
}

void writeFile(QString text, QString path)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << text;
        file.close();
    }
}

void tempSave(QString text, QString path)
{
    filesystem::path temp_path = path.toStdString();
    filesystem::create_directories(temp_path.parent_path());
    writeFile(text, path);
}

const QString tempOpen(QString path, QString tempPath)
{
    QFileInfo temp_info(tempPath);
    if (temp_info.exists())
        return readFile(tempPath);
    else
        return readFile(path);
}

bool checkChildStatus(QString possibleParent, QString possibleChild)
{
    filesystem::path parent = possibleParent.toStdString();
    filesystem::path child = possibleChild.toStdString();
    filesystem::path child_parent = child.parent_path();
    if (child_parent.make_preferred() == parent.make_preferred())
        return true;
    return false;
}

// io.cpp, fernanda