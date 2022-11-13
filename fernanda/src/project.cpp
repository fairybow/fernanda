// project.cpp, fernanda

#include "project.h"

Project::Project(QString filePath, SP opt)
{
	activeArchive = filePath;
	if (!Path::exists(filePath))
		makeProject(filePath, opt);
	auto xml = loadXml(filePath);
	dom->set(xml);
}

QVector<QString> Project::makeInitExpansions()
{
	QVector<QString> result;
	if (!dom->hasValue()) return result;
	auto expansions = dom->getElements("expanded", "true");
	for (auto& element : expansions)
		result << element.attribute("key");
	return result;
}

QVector<QStandardItem*> Project::makeItems()
{
	QVector<QStandardItem*> result;
	if (!dom->hasValue()) return result;
	QXmlStreamReader reader(dom->string());
	while (!reader.atEnd())
	{
		auto name = reader.name().toString();
		if (reader.isStartElement() && name == "root")
			reader.readNext();
		else if (reader.isStartElement() && name != "root")
		{
			auto item = makeItems_recursor(reader);
			result << item;
		}
		else
			reader.readNextStartElement();
	}
	return result;
}

const QString Project::getActiveKey()
{
	return activeKey;
}

void Project::autoTempSave(QString oldText)
{
	if (activeKey == nullptr) return;
	tempSave(activeKey, oldText);
}

const QString Project::saveOld_openNew(QString newKey, QString oldText)
{
	if (activeKey != nullptr && oldText != nullptr)
		tempSave(activeKey, oldText);
	return tempOpen(newKey);
}

QVector<QString> Project::handleEditsList(QString currentText)
{
	QVector<QString> result;
	if (activeKey == nullptr || cleanText == nullptr) return result;
	if (cleanText != currentText)
		Io::amendVector(edits_metaDoc, activeKey, Io::AmendVector::Add);
	else
		Io::amendVector(edits_metaDoc, activeKey, Io::AmendVector::Remove);
	result = edits_metaDoc;
	return result;
}

bool Project::hasAnyChanges()
{
	if (!edits_metaDoc.isEmpty() || dom->hasChanges()) return true;
	else return false;
}

void Project::saveProject(QString currentText)
{
	//
}

void Project::domMove(QString pivotKey, QString fulcrumKey, Io::Move pos)
{
	dom->moveElement(pivotKey, fulcrumKey, pos);
}

void Project::newDomElement(QString newName, Path::Type type, QString parentKey)
{
	dom->newElement(newName, type, parentKey);
}

void Project::renameDomElement(QString newName, QString key)
{
	dom->renameElement(newName, key);
}

void Project::makeProject(QString filePath, SP opt)
{
	QVector<Io::ArchivePaths> data_list;
	data_list << Io::ArchivePaths{ "story" };
	if (opt == SP::MakeSample)
		data_list << Sample::make();
	createArc(data_list, filePath);
}

void Project::createArc(QVector<Io::ArchivePaths> dataList, QString writePath)
{
	QTemporaryDir temp_dir;
	auto temp_path = temp_dir.path();
	for (auto& entry : dataList)
	{
		auto archive_path = entry.archivePath.toStdWString();
		auto temp_sub_path = (temp_path / entry.archivePath).toStdWString();
		if (!entry.readPath.has_value())
			Path::makeDirs(temp_sub_path);
		else
		{
			Path::makeDirs(std::filesystem::path(temp_sub_path).parent_path());
			auto q_temp_sub_path = QString::fromStdWString(temp_sub_path);
			QFile::copy(entry.readPath.value(), q_temp_sub_path);
			QFile(q_temp_sub_path).setPermissions(QFile::WriteUser);
		}
	}
	archiver->arc(Archiver::Op::Create, (temp_path / "story"), writePath);
}

void Project::addToArc(QVector<Io::ArchivePaths> dataList, QString filePath)
{
	std::map<std::wstring, std::wstring> files_map;
	for (auto& entry : dataList)
	{
		if (entry.readPath.has_value())
			files_map[entry.readPath.value().toStdWString()] = entry.archivePath.toStdWString();
	}
	archiver->arc(Archiver::Op::Add, files_map, filePath);
}

const QString Project::loadXml(QString filePath)
{
	QString result;
	result = readInArchive(filePath, "story.xml");
	if (result == nullptr)
	{
		newXml(filePath);
		result = readInArchive(filePath, "story.xml");
	}
	return result;
}

void Project::newXml(QString filePath)
{	
	QTemporaryDir dir;
	extractArc(filePath, dir.path());
	QString xml;
	QXmlStreamWriter writer(&xml);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("root");
	writer.writeAttribute("rel_path", "story");
	writer.writeAttribute("project", Path::getName(filePath));
	writer.writeAttribute("file_path", Path::makePreferred(filePath));
	newXml_recursor(writer, dir.path() / "story");
	writer.writeEndDocument();
	QTemporaryDir dir2;
	Io::writeFile(dir2.path() / "story.xml", xml);
	QVector<Io::ArchivePaths> data_list;
	data_list << Io::ArchivePaths{ "story.xml", dir2.path() / "story.xml" };
	addToArc(data_list, filePath);
}

void Project::newXml_recursor(QXmlStreamWriter& writer, QString readPath, QString rootPath)
{
	if (rootPath == nullptr)
		rootPath = readPath;
	QDirIterator it(readPath, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	while (it.hasNext())
	{
		it.next();
		auto rel_path = "story" / Path::relPath(rootPath, it.filePath());
		if (it.fileInfo().isDir())
		{
			writer.writeStartElement("dir");
			writer.writeAttribute("rel_path", rel_path);
			writer.writeAttribute("key", QUuid::createUuid().toString(QUuid::WithoutBraces));
			writer.writeAttribute("expanded", "false");
			newXml_recursor(writer, readPath / it.fileName(), rootPath);
			writer.writeEndElement();
		}
		else
		{
			writer.writeStartElement("file");
			writer.writeAttribute("rel_path", rel_path);
			writer.writeAttribute("key", QUuid::createUuid().toString(QUuid::WithoutBraces));
			writer.writeAttribute("expanded", "false");
			writer.writeEndElement();
		}
	}
}

void Project::extractArc(QString filePath, QString extractPath)
{
	archiver->arc(Archiver::Op::Extract, filePath, extractPath);
}

QStandardItem* Project::makeItems_recursor(QXmlStreamReader& reader)
{
	auto type = reader.name().toString();
	auto key = reader.attributes().value("key").toString();
	auto name = dom->elementName(key);
	auto result = new QStandardItem;
	result->setData(type, Qt::UserRole);
	result->setData(name, Qt::UserRole + 1);
	result->setData(key, Qt::UserRole + 2);
	reader.readNext();
	while (!reader.isEndElement())
	{
		if (reader.isStartElement())
		{
			auto child_item = makeItems_recursor(reader);
			result->appendRow(child_item);
		}
		else
			reader.readNext();
	}
	reader.readNext();
	return result;
}

void Project::tempSave(QString key, QString text)
{
	auto temp_file = tempPath(key);
	Io::writeFile(temp_file, text);
}

const QString Project::tempOpen(QString newKey)
{
	activeKey = newKey;
	auto tar_rel_path = dom->relPath(newKey);
	if (tar_rel_path != nullptr)
		cleanText = readInArchive(activeArchive, tar_rel_path);
	else
		cleanText = nullptr;
	auto temp_path = tempPath(newKey);
	QString result;
	if (QFile(temp_path).exists())
		result = Io::readFile(temp_path);
	else
		result = cleanText;
	return result;
}

const QString Project::readInArchive(QString filePath, QString relPath)
{
	QString result = nullptr;
	QTemporaryDir temp_dir;
	auto temp_dir_path = temp_dir.path();
	auto was_found = archiver->arc(Archiver::Op::Extract, filePath, relPath, temp_dir_path);
	if (was_found)
		result = Io::readFile(temp_dir_path / relPath);
	return result;
}

const QString Project::tempPath(QString key)
{
	auto rel_path = key + ".txt~";
	auto temps_dir = Ud::userData(Ud::Op::GetTemp);
	auto proj_temp = temps_dir / Path::getName(activeArchive);
	return proj_temp / rel_path;
}

bool Project::isEdited(QString key)
{
	if (edits_metaDoc.contains(key))
		return true;
	return false;
}

// project.cpp, fernanda