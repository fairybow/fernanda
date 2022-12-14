// dom.cpp, fernanda

#include "dom.h"

void Dom::set(QString xmlDoc)
{
	self.setContent(xmlDoc);
	initialSelf.setContent(string());
	cutElements.setContent(QString("<root/>"));
}

const QString Dom::string(Doc doc)
{
	QString result = nullptr;
	switch (doc) {
	case Doc::Current:
		result = self.toString();
		break;
	case Doc::Cuts:
		result = cutElements.toString();
		break;
	case Doc::Initial:
		result = initialSelf.toString();
		break;
	}
	return result;
}

bool Dom::hasChanges()
{
	if (string() != string(Doc::Initial)) return true;
	return false;
}

void Dom::move(QString pivotKey, QString fulcrumKey, Io::Move pos)
{
	auto pivot = element<QDomElement>(pivotKey);
	auto fulcrum = element<QDomElement>(fulcrumKey);
	QString new_pivot_path;
	QString new_pivot_parent_path;
	auto pivot_name = element<QString>(pivotKey, Element::Name);
	if (isFile(pivot))
		pivot_name = pivot_name + ".txt";
	switch (pos) {
	case Io::Move::Above:
		fulcrum.parentNode().insertBefore(pivot, fulcrum);
		movePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrumKey);
		break;
	case Io::Move::Below:
		fulcrum.parentNode().insertAfter(pivot, fulcrum);
		movePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrumKey);
		break;
	case Io::Move::On:
		fulcrum.appendChild(pivot);
		if (isDir(fulcrum))
		{
			auto fulcrum_path = element<QString>(fulcrumKey, Element::Path);
			new_pivot_path = fulcrum_path / pivot_name;
			new_pivot_parent_path = fulcrum_path;
		}
		else if (isFile(fulcrum))
			movePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrumKey);
		break;
	case Io::Move::Viewport:
		self.documentElement().appendChild(pivot);
		new_pivot_parent_path = "story";
		new_pivot_path = new_pivot_parent_path / pivot_name;
		break;
	}
	QString children_base_path;
	if (isDir(pivot))
		children_base_path = new_pivot_path;
	else if (isFile(pivot))
		children_base_path = new_pivot_parent_path;
	auto renames = prepareChildRenames_recursor(pivot, children_base_path);
	renames << Io::ArcRename{ pivotKey, new_pivot_path };
	for (auto& rename : renames)
		write(rename.key, rename.relPath, Write::Rename);
}

void Dom::rename(QString newName, QString key)
{
	if (newName == nullptr) return;
	auto elem = element<QDomElement>(key);
	if (isFile(elem))
		newName = newName + ".txt";
	auto parent_path = element<QString>(key, Element::ParentDirPath);
	auto new_path = parent_path / newName;
	QVector<Io::ArcRename> renames = { Io::ArcRename{ key, new_path } };
	if (isDir(elem))
		renames << prepareChildRenames_recursor(elem, new_path, ChildRenames::InPlace);
	for (auto& rename : renames)
		write(rename.key, rename.relPath, Write::Rename);
}

void Dom::add(QString newName, Path::Type type, QString parentKey)
{
	if (newName == nullptr) return;
	QString tag_name;
	QString name;
	switch (type) {
	case Path::Type::Dir:
		tag_name = "dir";
		name = newName;
		break;
	case Path::Type::File:
		tag_name = "file";
		name = newName + ".txt";
		break;
	}
	auto elem = self.createElement(tag_name);
	auto key = QUuid::createUuid().toString(QUuid::WithoutBraces);
	elem.setAttribute("key", key);
	elem.setAttribute("expanded", "false");
	QDomElement parent_element;
	QString nearest_dir = nullptr;
	if (parentKey != nullptr)
	{
		parent_element = element<QDomElement>(parentKey);
		if (isDir(parent_element))
			nearest_dir = element<QString>(parentKey, Element::Path);
		else
		{
			auto parent_dir_key = element<QString>(parentKey, Element::ParentDirKey);
			if (parent_dir_key != nullptr)
				nearest_dir = element<QString>(parent_dir_key, Element::Path);
		}
	}
	else
		parent_element = self.documentElement();
	if (nearest_dir == nullptr)
		nearest_dir = "story";
	auto path = nearest_dir / name;
	parent_element.appendChild(elem);
	write(key, path, Write::Rename);
}

void Dom::cut(QString key)
{
	auto elem = element<QDomElement>(key);
	cutElements.documentElement().appendChild(cutElements.importNode(elem, true));
	elem.parentNode().removeChild(elem);
}

QVector<Io::ArcRename> Dom::cuts()
{
	QVector<Io::ArcRename> result;
	auto cut_elements = elements(cutElements);
	for (auto& cut_element : cut_elements)
	{
		auto key = cut_element.attribute("key");
		auto rename = cut_element.attribute("rename");
		auto rel_path = cut_element.attribute("rel_path");
		Path::Type type{};
		(isDir(cut_element))
			? type = Path::Type::Dir
			: type = Path::Type::File;
		(rel_path.isEmpty())
			? result << Io::ArcRename{ key, rename, std::optional<QString>(), type }
			: result << Io::ArcRename{ key, (QString((rename.isEmpty()) ? rel_path : rename)), std::optional<QString>(rel_path), type};
	}
	return result;
}

QVector<Io::ArcRename> Dom::renames(Finalize finalize)
{
	QVector<Io::ArcRename> result;
	for (auto& renamed_element : elementsByAttribute("rename"))
	{
		auto key = renamed_element.attribute("key");
		auto rename = renamed_element.attribute("rename");
		auto rel_path = renamed_element.attribute("rel_path");
		if (rel_path == rename)
			renamed_element.removeAttribute("rename");
		else
		{
			if (rel_path.isEmpty())
			{
				Path::Type type{};
				(isDir(renamed_element))
					? type = Path::Type::Dir
					: type = Path::Type::File;
				result << Io::ArcRename{ key, rename, std::optional<QString>(), type };
			}
			else
				result << Io::ArcRename{ key, rename, rel_path };
			if (finalize == Finalize::Yes)
			{
				renamed_element.setAttribute("rel_path", rename);
				renamed_element.removeAttribute("rename");
			}
		}
	}
	return result;
}

QDomElement Dom::element_recursor(QDomElement node, QString key, QDomElement result)
{
	if (node.attribute("key") == key)
		result = node;
	auto child_node = node.firstChildElement();
	while (!child_node.isNull())
	{
		auto elem = element_recursor(child_node, key);
		if (!elem.isNull())
			result = elem;
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

QVector<QDomElement> Dom::elements(QDomDocument doc)
{
	QVector<QDomElement> result;
	auto root = doc.documentElement();
	auto next_node = root.firstChildElement();
	while (!next_node.isNull())
	{
		result << elements_recursor(next_node);
		next_node = next_node.nextSiblingElement();
	}
	return result;
}

QVector<QDomElement> Dom::elements_recursor(QDomElement node, QVector<QDomElement> result)
{
	if (node.isElement())
		result << node;
	auto child_node = node.firstChildElement();
	while (!child_node.isNull())
	{
		result << elements_recursor(child_node);
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

QVector<QDomElement> Dom::elementsByAttribute(QString attribute, QString value)
{
	QVector<QDomElement> result;
	auto root = self.documentElement();
	auto next_node = root.firstChildElement();
	while (!next_node.isNull())
	{
		result << elementsByAttribute_recursor(next_node, attribute, value);
		next_node = next_node.nextSiblingElement();
	}
	return result;
}

QVector<QDomElement> Dom::elementsByAttribute_recursor(QDomElement node, QString attribute, QString value, QVector<QDomElement> result)
{
	if (value != nullptr)
	{
		if (node.hasAttribute(attribute) && node.attribute(attribute) == value)
			result << node;
	}
	else
	{
		if (node.hasAttribute(attribute))
			result << node;
	}
	auto child_node = node.firstChildElement();
	while (!child_node.isNull())
	{
		result << elementsByAttribute_recursor(child_node, attribute, value);
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

void Dom::movePaths(QString& newPivotPath, QString& newPivotParentPath, QString pivotName, QString fulcrumKey)
{
	auto fulcrum_parent_key = element<QString>(fulcrumKey, Element::ParentDirKey);
	(fulcrum_parent_key == nullptr)
		? newPivotParentPath = "story"
		: newPivotParentPath = element<QString>(fulcrum_parent_key, Element::Path);
	newPivotPath = newPivotParentPath / pivotName;
}

QVector<Io::ArcRename> Dom::prepareChildRenames_recursor(QDomElement node, QString stemPathParent, ChildRenames renameType, QVector<Io::ArcRename> result)
{
	auto child = node.firstChildElement();
	while (!child.isNull())
	{
		auto child_key = child.attribute("key");
		QString child_name;
		if (isDir(child))
			child_name = element<QString>(child_key, Element::Name);
		else if (isFile(child))
			child_name = element<QString>(child_key, Element::Name) + ".txt";
		auto nearest_dir_key = element<QString>(child_key, Element::ParentDirKey);
		auto nearest_dir_name = element<QString>(nearest_dir_key, Element::Name);
		auto stem_path_name = Path::getName(stemPathParent);
		QString next_stem_path;
		if (stem_path_name == nearest_dir_name || renameType == ChildRenames::InPlace)
			next_stem_path = stemPathParent;
		else
			next_stem_path = stemPathParent / nearest_dir_name;
		auto rel_path = next_stem_path / child_name;
		result << Io::ArcRename{ child_key, rel_path };
		result << prepareChildRenames_recursor(child, next_stem_path, renameType);
		child = child.nextSiblingElement();
	}
	return result;
}

QString Dom::filterPath(QDomElement elem, Filter filter)
{
	QString result = nullptr;
	switch (filter) {
	case Filter::OrigToNullptr:
		if (elem.hasAttribute("rel_path"))
			result = elem.attribute("rel_path");
		break;
	case Filter::RenameToOrig:
		(elem.hasAttribute("rename"))
			? result = elem.attribute("rename")
			: result = elem.attribute("rel_path");
		break;
	}
	return result;
}

// dom.cpp, fernanda