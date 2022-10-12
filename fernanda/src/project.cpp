// project.cpp, fernanda

#include "project.h"

Project::Project(QString filePath, SP opt) // temporarily receives a dir name
{
	auto file_path = filePath.toStdString();
	auto exists = IO::checkExists(filePath);
	if (!exists)
		makeProject(file_path, opt);
	auto xml = loadXml(file_path);
	setProjectMap(xml);
}

QVector<QStandardItem*> Project::items()
{
	QVector<QStandardItem*> result;
	if (!map.has_value()) return result;
	QXmlStreamReader reader(map.value());
	while (!reader.atEnd())
	{
		auto name = reader.name().toString();
		if (reader.isStartElement() && name == "root")
		{
			currentProjectName = reader.attributes().value("project").toString();
			currentRoot = reader.attributes().value("path").toString();
			reader.readNext();
		}
		else if (reader.isStartElement() && name != "root")
		{
			auto item = items_recursor(reader);
			result << item;
		}
		else
			reader.readNextStartElement();
	}
	return result;
}

const QString Project::handleIO(QString newFileRelPath, QString oldText)
{
	if (!map.has_value()) return nullptr;
	if (!currentProjectName.has_value() || !currentRoot.has_value()) return nullptr;
	if (currentRelFilePath.has_value())
		tempSave(currentRelFilePath.value(), oldText);
	return handleNewFile(newFileRelPath);
}

QVector<QString> Project::handleEditedList(QString currentText)
{
	QVector<QString> result;
	auto& path = currentRelFilePath.value();
	if (!cleanText.has_value()) return result;
	if (cleanText.value() != currentText)
	{
		if (!editedFiles.contains(path))
			editedFiles << path;
	}
	else
	{
		if (editedFiles.contains(path))
			editedFiles.removeAll(path);
	}
	result = editedFiles;
	return result;
}

QString Project::getCurrentRelPath()
{
	if (!currentRelFilePath.has_value()) return nullptr;
	return currentRelFilePath.value();
}

void Project::saveCurrent(QString text)
{
	if (!currentRelFilePath.has_value()) return;
	auto& current_path = currentRelFilePath.value();
	std::filesystem::path main = derivePath(current_path, PT::Main).toStdString();
	std::filesystem::path temp = derivePath(current_path, PT::Temp).toStdString();
	std::filesystem::path rollback = derivePath(current_path, PT::Rollback).toStdString();
	tempSave(current_path, text);
	if (QFileInfo(rollback).exists())
        std::filesystem::remove(rollback);
    else
        std::filesystem::create_directories(rollback.parent_path());
    std::filesystem::rename(main, rollback);
    std::filesystem::rename(temp, main);
	cleanText = IO::readFile(QString::fromStdString(main.string()));
}

void Project::saveNonActiveFiles()
{
	if (editedFiles.isEmpty()) return;
	for (auto& rel_path : editedFiles)
	{
		std::filesystem::path main = derivePath(rel_path, PT::Main).toStdString();
		std::filesystem::path temp = derivePath(rel_path, PT::Temp).toStdString();
		std::filesystem::path rollback = derivePath(rel_path, PT::Rollback).toStdString();
		if (QFileInfo(rollback).exists())
			std::filesystem::remove(rollback);
		else
			std::filesystem::create_directories(rollback.parent_path());
		std::filesystem::rename(main, rollback);
		std::filesystem::rename(temp, main);
	}
	editedFiles.clear();
}

bool Project::hasEdits()
{
	if (!editedFiles.isEmpty()) return true;
	else return false;
}

void Project::createFile(std::filesystem::path relPath)
{
	//
}

void Project::editFile(std::filesystem::path relPath, QString content)
{
	//
}

void Project::deleteFile(std::filesystem::path relPath)
{
	//
}

void Project::createFolder(std::filesystem::path relPath)
{
	//
}

void Project::deleteFolder(std::filesystem::path relPath)
{
	//
}

void Project::rename(std::filesystem::path relPath, std::string newName)
{
	//
}

void Project::reparent(std::filesystem::path relPath, std::filesystem::path newPath)
{
	//
}

void Project::reorder(std::filesystem::path relPath, int nthChild)
{
	//
}

void Project::tests(QString str)
{
	//
}

void Project::makeProject(std::filesystem::path path, SP opt)
{
	std::filesystem::create_directories(path / "story");
	if (opt == SP::MakeSample)
		Sample::make(path / "story");
	makeXml(path);
}

void Project::makeXml(std::filesystem::path writePath, std::filesystem::path targetPath)
{
	auto xml_path = writePath / "story.xml";
	auto q_xml_path = QString::fromStdString(xml_path.string());
	if (targetPath.empty())
		targetPath = writePath / "story";
	QString xml;
	QXmlStreamWriter writer(&xml);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("root");
	writer.writeAttribute("project", IO::getName(QString::fromStdString(writePath.string())));
	writer.writeAttribute("path", QString::fromStdString(targetPath.make_preferred().string()));
	makeXml_recursor(writer, targetPath);
	writer.writeEndDocument();
	IO::writeFile(q_xml_path, xml);
}

void Project::makeXml_recursor(QXmlStreamWriter& writer, std::filesystem::path readPath, std::filesystem::path rootPath)
{
	if (rootPath.empty())
		rootPath = readPath;
	QDirIterator it(QString::fromStdString(readPath.string()), QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	while (it.hasNext())
	{
		it.next();
		auto rel_path = IO::relPath(rootPath, it.filePath().toStdString());
		if (it.fileInfo().isDir())
		{
			writer.writeStartElement("dir");
			writer.writeAttribute("path", rel_path); //writer.writeAttribute("path", it.filePath());
			makeXml_recursor(writer, readPath / it.fileName().toStdString(), rootPath);
			writer.writeEndElement();
		}
		else
		{
			writer.writeStartElement("file");
			writer.writeAttribute("path", rel_path); //writer.writeAttribute("path", it.filePath());
			writer.writeEndElement();
		}
	}
}

QString Project::loadXml(std::filesystem::path path)
{
	auto xml_path = QString::fromStdString((path / "story.xml").string());
	auto exists = IO::checkExists(xml_path);
	if (!exists)
		makeXml(path);
	auto result = IO::readFile(xml_path);
	return result;
}

void Project::setProjectMap(QString xml) // handle the intermediate stage here, in future
{
	map = xml;
}

QStandardItem* Project::items_recursor(QXmlStreamReader& reader)
{
	auto type = reader.name().toString();
	auto path = reader.attributes().value("path").toString();
	auto name = IO::getName(path);
	auto result = new QStandardItem;
	result->setData(type, Qt::UserRole);
	result->setData(path, Qt::UserRole + 1);
	result->setData(name, Qt::UserRole + 2);
	reader.readNext();
	while (!reader.isEndElement())
	{
		if (reader.isStartElement())
		{
			auto child_item = items_recursor(reader);
			result->appendRow(child_item);
		}
		else
			reader.readNext();
	}
	reader.readNext();
	return result;
}

void Project::tempSave(QString relFilePath, QString text)
{
	auto temp_file = derivePath(relFilePath, PT::Temp);
	std::filesystem::path tmp = temp_file.toStdString();
	auto parent = tmp.parent_path();
	UD::makeDirs(QString::fromStdString(parent.string()));
	IO::writeFile(temp_file, text);
}

const QString Project::handleNewFile(QString relFilePath)
{
	currentRelFilePath = relFilePath;
	QString result;
	cleanText = IO::readFile(derivePath(relFilePath, PT::Main));
	auto temp_file = derivePath(relFilePath, PT::Temp);
	if (QFile(temp_file).exists())
		result = IO::readFile(temp_file);
	else
		result = cleanText.value();
	return result;
}

const QString Project::derivePath(QString relFilePath, PT type)
{
	QString result;
	switch (type) {
		case PT::Main:
			result = currentRoot.value() + "/" + relFilePath;
			break;
		case PT::Temp:
		{
			auto temps_dir = UD::userData(UD::Op::GetTemp);
			auto proj_temp = temps_dir + "/" + currentProjectName.value() + "/story";
			result = proj_temp + "/" + relFilePath + "~";
			break;
		}
		case PT::Rollback:
		{
			auto rb_dir = UD::userData(UD::Op::GetRollback);
			auto proj_rb = rb_dir + "/" + currentProjectName.value() + "/story";
			result = proj_rb + "/" + relFilePath + "~";
			break;
		}
	}
	return result;
}

// project.cpp, fernanda