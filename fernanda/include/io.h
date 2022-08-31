#pragma once

#include <filesystem>

#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QString>
#include <QTextStream>

using namespace std;

const QString readFile(QString path);
void writeFile(QString text, QString path);
void tempSave(QString text, QString path);
const QString tempOpen(QString path, QString tempPath);
bool checkChildStatus(QString possibleParent, QString possibleChild);

// io.h, fernanda