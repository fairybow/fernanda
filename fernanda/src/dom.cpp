// dom.cpp, fernanda

#include "dom.h"

void Dom::set(QString xmlDoc)
{
	self.setContent(xmlDoc);
	initialSelf.setContent(string());
	cutElements.setContent(QStringLiteral("<root/>"));
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

void Dom::move(QString pivotKey, QString fulcrumKey, Io::Move position)
{
	auto pivot = element<QDomElement>(pivotKey);
	auto fulcrum = element<QDomElement>(fulcrumKey);
	FsPath new_pivot_path;
	FsPath new_pivot_parent_path;
	auto pivot_name = element<QString>(pivotKey, Element::Name);
	if (isFile(pivot))
		pivot_name = pivot_name + Io::ext;
	switch (position) {
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
			auto fulcrum_path = element<FsPath>(fulcrumKey, Element::Path);
			new_pivot_path = fulcrum_path / Path::toFs(pivot_name);
			new_pivot_parent_path = fulcrum_path;
		}
		else if (isFile(fulcrum))
			movePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrumKey);
		break;
	case Io::Move::Viewport:
		self.documentElement().appendChild(pivot);
		new_pivot_parent_path = Io::storyRoot;
		new_pivot_path = new_pivot_parent_path / Path::toFs(pivot_name);
		break;
	}
	FsPath children_base_path;
	if (isDir(pivot))
		children_base_path = new_pivot_path;
	else if (isFile(pivot))
		children_base_path = new_pivot_parent_path;
	auto renames = prepareChildRenames_recursor(pivot, children_base_path);
	renames << Io::ArcRename{ pivotKey, new_pivot_path };
	for (auto& rename : renames)
		write(rename.key, Path::toQString(rename.relPath, true), Write::Rename);
}

void Dom::rename(QString newName, QString key)
{
	if (newName == nullptr) return;
	auto elem = element<QDomElement>(key);
	if (isFile(elem))
		newName = newName + Io::ext;
	auto parent_path = element<FsPath>(key, Element::ParentDirPath);
	auto new_path = parent_path / Path::toFs(newName);
	QVector<Io::ArcRename> renames = { Io::ArcRename{ key, new_path } };
	if (isDir(elem))
		renames << prepareChildRenames_recursor(elem, new_path, ChildRenames::InPlace);
	for (auto& rename : renames)
		write(rename.key, Path::toQString(rename.relPath, true), Write::Rename);
}

void Dom::add(QString newName, Path::Type type, QString parentKey)
{
	if (newName == nullptr) return;
	QString tag_name;
	QString name;
	switch (type) {
	case Path::Type::Dir:
		tag_name = tagDir;
		name = newName;
		break;
	case Path::Type::File:
		tag_name = tagFile;
		name = newName + Io::ext;
		break;
	}
	auto elem = self.createElement(tag_name);
	auto key = QUuid::createUuid().toString(QUuid::WithoutBraces);
	elem.setAttribute(attrKey, key);
	elem.setAttribute(attrExpanded, "false");
	QDomElement parent_element;
	FsPath nearest_dir;
	if (parentKey != nullptr)
	{
		parent_element = element<QDomElement>(parentKey);
		if (isDir(parent_element))
			nearest_dir = element<FsPath>(parentKey, Element::Path);
		else
		{
			auto parent_dir_key = element<QString>(parentKey, Element::ParentDirKey);
			if (parent_dir_key != nullptr)
				nearest_dir = element<FsPath>(parent_dir_key, Element::Path);
		}
	}
	else
		parent_element = self.documentElement();
	if (nearest_dir.empty())
		nearest_dir = Io::storyRoot;
	auto path = nearest_dir / Path::toFs(name);
	parent_element.appendChild(elem);
	write(key, Path::toQString(path, true), Write::Rename);
}

QVector<QString> Dom::cut(QString key)
{
	QVector<QString> result;
	result << key;
	auto elem = element<QDomElement>(key);
	result << childKeys_recursor(elem);
	cutElements.documentElement().appendChild(cutElements.importNode(elem, true));
	elem.parentNode().removeChild(elem);
	return result;
}

QVector<Io::ArcRename> Dom::cuts()
{
	QVector<Io::ArcRename> result;
	auto cut_elements = elements(cutElements);
	for (auto& cut_element : cut_elements)
	{
		auto key = cut_element.attribute(attrKey);
		auto rename = Path::toFs(cut_element.attribute(attrRename));
		auto rel_path = Path::toFs(cut_element.attribute(attrRelPath));
		Path::Type type{};
		(isDir(cut_element))
			? type = Path::Type::Dir
			: type = Path::Type::File;
		(rel_path.empty())
			? result << Io::ArcRename{ key, rename, std::optional<FsPath>(), type }
			: result << Io::ArcRename{ key, (rename.empty() ? rel_path : rename), std::optional<FsPath>(rel_path), type};
	}
	return result;
}

QVector<Io::ArcRename> Dom::renames(Finalize finalize)
{
	QVector<Io::ArcRename> result;
	for (auto& renamed_element : elementsByAttribute(attrRename))
	{
		auto key = renamed_element.attribute(attrKey);
		auto rename = Path::toFs(renamed_element.attribute(attrRename));
		auto rel_path = Path::toFs(renamed_element.attribute(attrRelPath));
		if (rel_path == rename)
			renamed_element.removeAttribute(attrRename);
		else
		{
			if (rel_path.empty())
			{
				Path::Type type{};
				(isDir(renamed_element))
					? type = Path::Type::Dir
					: type = Path::Type::File;
				result << Io::ArcRename{ key, rename, std::optional<FsPath>(), type };
			}
			else
				result << Io::ArcRename{ key, rename, rel_path };
			if (finalize == Finalize::Yes)
			{
				renamed_element.setAttribute(attrRelPath, Path::toQString(rename));
				renamed_element.removeAttribute(attrRename);
			}
		}
	}
	return result;
}

QDomElement Dom::element_recursor(QDomElement node, QString key, QDomElement result)
{
	if (node.attribute(attrKey) == key)
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

void Dom::movePaths(FsPath& newPivotPath, FsPath& newPivotParentPath, QString pivotName, QString fulcrumKey)
{
	auto fulcrum_parent_key = element<QString>(fulcrumKey, Element::ParentDirKey);
	(fulcrum_parent_key == nullptr)
		? newPivotParentPath = Io::storyRoot
		: newPivotParentPath = element<FsPath>(fulcrum_parent_key, Element::Path);
	newPivotPath = newPivotParentPath / Path::toFs(pivotName);
}

QVector<QString> Dom::childKeys_recursor(QDomElement node, QVector<QString> result)
{
	auto child_node = node.firstChildElement();
	while (!child_node.isNull())
	{
		if (isFile(child_node))
			result << child_node.attribute(attrKey);
		result << childKeys_recursor(child_node);
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

QVector<Io::ArcRename> Dom::prepareChildRenames_recursor(QDomElement node, FsPath stemPathParent, ChildRenames renameType, QVector<Io::ArcRename> result)
{
	auto child = node.firstChildElement();
	while (!child.isNull())
	{
		auto child_key = child.attribute(attrKey);
		QString child_name;
		if (isDir(child))
			child_name = element<QString>(child_key, Element::Name);
		else if (isFile(child))
			child_name = element<QString>(child_key, Element::Name) + Io::ext;
		auto nearest_dir_key = element<QString>(child_key, Element::ParentDirKey);
		auto nearest_dir_name = element<QString>(nearest_dir_key, Element::Name);
		auto stem_path_name = Path::getName<QString>(stemPathParent);
		FsPath next_stem_path;
		if (stem_path_name == nearest_dir_name || renameType == ChildRenames::InPlace)
			next_stem_path = stemPathParent;
		else
			next_stem_path = stemPathParent / Path::toFs(nearest_dir_name);
		auto rel_path = next_stem_path / Path::toFs(child_name);
		result << Io::ArcRename{ child_key, rel_path };
		result << prepareChildRenames_recursor(child, next_stem_path, renameType);
		child = child.nextSiblingElement();
	}
	return result;
}

Dom::FsPath Dom::filterPath(QDomElement elem, Filter filter)
{
	FsPath result;
	switch (filter) {
	case Filter::OrigToNullptr:
		if (elem.hasAttribute(attrRelPath))
			result = Path::toFs(elem.attribute(attrRelPath));
		break;
	case Filter::RenameToOrig:
		(elem.hasAttribute(attrRename))
			? result = Path::toFs(elem.attribute(attrRename))
			: result = Path::toFs(elem.attribute(attrRelPath));
		break;
	}
	return result;
}

// dom.cpp, fernanda