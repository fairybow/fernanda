// story.cpp, fernanda

#include "story.h"

Story::Story(QString filePath, Op opt)
{
	activeArcPath = filePath;
	if (!QFile(activeArcPath).exists())
		make(opt);
	dom->set(xml());
}

const QString Story::devGetDom(Dom::Doc doc)
{
	return dom->string(doc);
}

QVector<Io::ArcRename> Story::devGetRenames()
{
	return dom->renames();
}

QVector<QStandardItem*> Story::items()
{
	QVector<QStandardItem*> result;
	QXmlStreamReader reader(dom->string());
	while (!reader.atEnd())
	{
		auto name = reader.name().toString();
		if (reader.isStartElement() && name == "root")
			reader.readNext();
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

const QString Story::key()
{
	return activeKey;
}

const QString Story::tempSaveOld_openNew(QString newKey, QString oldText)
{
	if (activeKey != nullptr)
		tempSave(activeKey, oldText);
	return tempOpen(newKey);
}

void Story::autoTempSave(QString text)
{
	if (activeKey == nullptr) return;
	tempSave(activeKey, text);
}

QVector<QString> Story::edits(QString currentText)
{
	QVector<QString> result;
	if (activeKey == nullptr) return result;
	(cleanText != currentText)
		? amendEditsList(AmendEdits::Add)
		: amendEditsList(AmendEdits::Remove);
	result = editedKeys;
	return result;
}

bool Story::hasChanges()
{
	if (!editedKeys.isEmpty() || dom->hasChanges()) return true;
	return false;
}

void Story::setItemExpansion(QString key, bool isExpanded)
{
	dom->write(key, isExpanded, Dom::Write::Expanded);
}

void Story::move(QString pivotKey, QString fulcrumKey, Io::Move pos)
{
	dom->move(pivotKey, fulcrumKey, pos);
}

void Story::rename(QString newName, QString key)
{
	dom->rename(newName, key);
}

void Story::add(QString newName, Path::Type type, QString parentKey)
{
	dom->add(newName, type, parentKey);
}

void Story::cut(QString key)
{
	dom->cut(key);
}

void Story::save(QString text)
{
	bak();
	if (activeKey != nullptr && isEdited(activeKey))
		tempSave(activeKey, text);
	auto cuts = dom->cuts();
	auto renames = dom->renames(Dom::Finalize::Yes);
	if (!cuts.isEmpty())
		archiver->cut(activeArcPath, cuts);
	if (!renames.isEmpty())
		archiver->save(activeArcPath, renames);
	if (!editedKeys.isEmpty())
	{
		QVector<Io::ArcWRPaths> edits;
		for (auto& edited_key : editedKeys)
			edits << Io::ArcWRPaths{ dom->element<QString>(edited_key, Dom::Element::Path), tempPath(edited_key) };
		archiver->add(activeArcPath, edits);
		editedKeys.clear();
		tempOpen(activeKey);
	}
	archiver->add(activeArcPath, Io::ArcWrite{ dom->string(), "story.xml" });
	dom->set(xml());
}

void Story::make(Op opt)
{
	QVector<Io::ArcWRPaths> wr_paths;
	wr_paths << Io::ArcWRPaths{ "story" };
	if (opt == Op::Sample)
		wr_paths << Sample::make();
	archiver->create(activeArcPath, wr_paths);
}

const QString Story::xml()
{
	QString result;
	result = archiver->read(activeArcPath, "story.xml");
	if (result == nullptr)
	{
		newXml();
		result = archiver->read(activeArcPath, "story.xml");
	}
	return result;
}

void Story::newXml()
{
	QTemporaryDir temp_dir;
	auto temp_dir_path = temp_dir.path();
	QString xml;
	archiver->extract(activeArcPath, temp_dir_path);
	QXmlStreamWriter writer(&xml);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeStartElement("root");
	writer.writeAttribute("rel_path", "story");
	writer.writeAttribute("project", Path::getName(activeArcPath));
	writer.writeAttribute("file_path", Path::sanitize(activeArcPath));
	newXml_recursor(writer, temp_dir_path / "story");
	writer.writeEndDocument();
	QTemporaryDir temp_dir_2;
	auto temp_dir_2_path = temp_dir_2.path();
	auto temp_xml_path = temp_dir_2_path / "story.xml";
	Io::writeFile(temp_xml_path, xml);
	archiver->add(activeArcPath, temp_xml_path, "story.xml");
}

void Story::newXml_recursor(QXmlStreamWriter& writer, QString rPath, QString rootPath)
{
	if (rootPath == nullptr)
		rootPath = rPath;
	QDirIterator it(rPath, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
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
			newXml_recursor(writer, rPath / it.fileName(), rootPath);
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

QStandardItem* Story::items_recursor(QXmlStreamReader& reader)
{
	auto type = reader.name().toString();
	auto key = reader.attributes().value("key").toString();
	auto name = dom->element<QString>(key, Dom::Element::Name);
	auto expanded = reader.attributes().value("expanded").toString();
	auto result = new QStandardItem;
	result->setData(type, Qt::UserRole);
	result->setData(key, Qt::UserRole + 1);
	result->setData(name, Qt::UserRole + 2);
	result->setData(expanded, Qt::UserRole + 3);
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

void Story::tempSave(QString key, QString text)
{
	auto temp_file = tempPath(key);
	Io::writeFile(temp_file, text);
}

const QString Story::tempOpen(QString newKey)
{
	activeKey = newKey;
	auto arc_r_path = dom->element<QString>(newKey, Dom::Element::OrigPath);
	(arc_r_path != nullptr)
		? cleanText = archiver->read(activeArcPath, arc_r_path)
		: cleanText = nullptr;
	auto temp_path = tempPath(newKey);
	QString result;
	(QFile(temp_path).exists())
		? result = Io::readFile(temp_path)
		: result = cleanText;
	return result;
}

const QString Story::tempPath(QString key)
{
	auto rel_path = key + ".txt~";
	auto temps_dir = Ud::userData(Ud::Op::GetTemp);
	auto proj_temp = temps_dir / Path::getName(activeArcPath);
	return proj_temp / rel_path;
}

void Story::amendEditsList(AmendEdits op)
{
	switch (op) {
	case AmendEdits::Add:
		if (!editedKeys.contains(activeKey))
			editedKeys << activeKey;
		break;
	case AmendEdits::Remove:
		if (editedKeys.contains(activeKey))
			editedKeys.removeAll(activeKey);
		break;
	}
}

bool Story::isEdited(QString key)
{
	if (editedKeys.contains(key)) return true;
	return false;
}

void Story::bak()
{
	auto timestamp = Ud::timestamp().replace(Uni::regex(Uni::Re::Forbidden), "_").replace(QRegularExpression("(\\s)"), "_").replace(QRegularExpression("(\\n)"), nullptr).toLower();
	timestamp.replace(QRegularExpression("(__)"), "_").replace(QRegularExpression("(_$)"), nullptr);
	auto bak_file_name = Path::getName(activeArcPath) + ".story." + timestamp + ".bak";
	auto bak_path = Ud::userData(Ud::Op::GetRollback) / bak_file_name;
	auto qf_bak = QFile(bak_path);
	if (qf_bak.exists())
		qf_bak.moveToTrash();
	QFile::copy(activeArcPath, bak_path);
}

// story.cpp, fernanda