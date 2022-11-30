// dom.cpp, fernanda

#include "dom.h"

void Dom::set(QString xml)
{
	self.setContent(xml);
	initialSelf.setContent(xml);
}

bool Dom::hasValue()
{
	if (!self.toString().isEmpty() || !self.toString().isNull()) return true;
	return false;
}

const QString Dom::string()
{
	return self.toString();
}

bool Dom::hasChanges()
{
	if (!renames_metaDoc.isEmpty()) return true;
	return false;
}

const QString Dom::relPath(QString key)
{
	auto element = findElement(key);
	if (element.hasAttribute("rel_path"))
		return element.attribute("rel_path");
	return nullptr;
}

const QString Dom::filteredRelPath(QString key)
{
	for (auto& item : renames_metaDoc)
		if (key == item.key)
			return item.changedRelPath;
	return relPath(key);
}

QVector<QDomElement> Dom::getElements(QString attribute, QString value)
{
	QVector<QDomElement> result;
	auto root = self.documentElement();
	auto next_node = root.firstChildElement();
	while (!next_node.isNull())
	{
		result << getElements_recursor(next_node, attribute, value);
		next_node = next_node.nextSiblingElement();
	}
	return result;
}

const QString Dom::elementName(QString key)
{
	QString result;
	if (isChanged(key))
	{
		for (auto& item : renames_metaDoc)
			if (key == item.key)
				result = Path::getName(item.changedRelPath);
	}
	else
		result = Path::getName(relPath(key));
	return result;
}

void Dom::moveElement(QString pivotKey, QString fulcrumKey, Io::Move pos) // clean me!
{
	//auto& dom = activeDom.value();
	if (pivotKey == nullptr) return;
	auto pivot_node = findElement(pivotKey);
	auto fulcrum_node = findElement(fulcrumKey);
	QString new_pivot_path;
	QString new_pivot_parent_path;
	QString pivot_name = Path::getName(filteredRelPath(pivotKey));
	if (pivot_node.tagName() == "file")
		pivot_name = pivot_name + ".txt";
	switch (pos) {
	case Io::Move::Above:
		fulcrum_node.parentNode().insertBefore(pivot_node, fulcrum_node);
		makeMovePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrum_node);
		break;
	case Io::Move::Below:
		fulcrum_node.parentNode().insertAfter(pivot_node, fulcrum_node);
		makeMovePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrum_node);
		break;
	case Io::Move::On:
		fulcrum_node.appendChild(pivot_node);
		if (fulcrum_node.tagName() == "dir")
		{
			auto fulcrum_path = filteredRelPath(fulcrumKey);
			new_pivot_path = fulcrum_path / pivot_name;
			new_pivot_parent_path = fulcrum_path;
		}
		else if (fulcrum_node.tagName() == "file")
			makeMovePaths(new_pivot_path, new_pivot_parent_path, pivot_name, fulcrum_node);
		break;
	case Io::Move::Viewport:
		self.documentElement().appendChild(pivot_node);
		new_pivot_parent_path = "story";
		new_pivot_path = new_pivot_parent_path / pivot_name;
		break;
	}
	QString children_base_path;
	if (pivot_node.tagName() == "dir")
		children_base_path = new_pivot_path;
	else if (pivot_node.tagName() == "file")
		children_base_path = new_pivot_parent_path;
	auto children = getElementChildTree_recursor(pivot_node, children_base_path);
	children << MetaDocRename{ pivotKey, new_pivot_path };
	handleRenamesList(children);
}

void Dom::newElement(QString newName, Path::Type type, QString parentKey)
{
	if (newName == nullptr) return;
	//if (!activeDom.has_value()) return;
	//auto& dom = activeDom.value();
	QString tag_name;
	QString file_name;
	switch (type) {
	case Path::Type::Dir:
		tag_name = "dir";
		file_name = newName;
		break;
	case Path::Type::File:
		tag_name = "file";
		file_name = newName + ".txt";
		break;
	}
	auto element = self.createElement(tag_name);
	auto key = QUuid::createUuid().toString(QUuid::WithoutBraces);
	element.setAttribute("key", key);
	element.setAttribute("expanded", "false");
	QDomElement parent_element;
	QString nearest_dir;
	if (parentKey != nullptr)
	{
		parent_element = findElement(parentKey);
		if (parent_element.tagName() == "dir")
			nearest_dir = filteredRelPath(parentKey);
		else
		{
			auto nearest_dir_key = nearestParentKeyOfType_recursor(parent_element, Path::Type::Dir);
			nearest_dir = filteredRelPath(nearest_dir_key);
		}
	}
	else
	{
		parent_element = self.documentElement();
		nearest_dir = "story";
	}
	auto path = nearest_dir / file_name;
	renames_metaDoc << MetaDocRename{ key, path };
	parent_element.appendChild(element);
}

void Dom::renameElement(QString newName, QString key)
{
	// make the new path
	if (newName == nullptr) return;
	//if (!activeDom.has_value()) return;
	auto element = findElement(key);
	auto nearest_dir_key = nearestParentKeyOfType_recursor(element, Path::Type::Dir);
	auto nearest_dir = filteredRelPath(nearest_dir_key);
	if (element.tagName() == "file")
		newName = newName + ".txt";
	auto new_path = nearest_dir / newName;
	QVector<MetaDocRename> renames;
	renames << MetaDocRename{ key, new_path };
	if (element.tagName() == "dir")
	{
		auto children = getElementChildTree_recursor(element, new_path);
		renames << children;
	}
	handleRenamesList(renames);
}

void Dom::setElementExpansionState(QString key, bool isExpanded)
{
	auto element = findElement(key);
	element.setAttribute("expanded", QString(isExpanded ? "true" : "false"));
}

bool Dom::isChanged(QString key)
{
	auto result = false;
	for (auto& item : renames_metaDoc)
		if (key == item.key)
			result = true;
	return result;
}

QDomElement Dom::findElement(QString key)
{
	QDomElement result;
	if (key == nullptr) return result;
	auto root = self.documentElement();
	auto next_node = root.firstChildElement();
	while (!next_node.isNull())
	{
		auto element = findElement_recursor(next_node, key);
		if (!element.isNull())
			result = element;
		next_node = next_node.nextSiblingElement();
	}
	return result;
}

QDomElement Dom::findElement_recursor(QDomElement node, QString key, QDomElement result)
{
	if (node.attribute("key") == key)
		result = node;
	auto child_node = node.firstChildElement();
	while (!child_node.isNull())
	{
		auto element = findElement_recursor(child_node, key);
		if (!element.isNull())
			result = element;
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

QVector<QDomElement> Dom::getElements_recursor(QDomElement node, QString attribute, QString value, QVector<QDomElement> result)
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
		result << getElements_recursor(child_node, attribute, value);
		child_node = child_node.nextSiblingElement();
	}
	return result;
}

QString Dom::nearestParentKeyOfType_recursor(QDomElement node, Path::Type type, QString result)
{
	auto parent = node.parentNode();
	QString name;
	switch (type) {
	case Path::Type::Dir:
		name = "dir";
		break;
	case Path::Type::File:
		name = "file";
		break;
	}
	if (parent.isElement() && parent.nodeName() == name)
		result = parent.toElement().attribute("key");
	else if (parent.isElement() && parent.nodeName() == "root")
		result = nullptr;
	else
		result = nearestParentKeyOfType_recursor(parent.toElement(), type);
	return result;
}

QVector<Dom::MetaDocRename> Dom::getElementChildTree_recursor(QDomElement node, QString stemPathParent, QVector<MetaDocRename> result)
{
	auto child = node.firstChildElement();
	while (!child.isNull())
	{
		auto child_key = child.attribute("key");
		QString child_name;
		if (child.tagName() == "dir")
			child_name = Path::getName(filteredRelPath(child_key));
		else if (child.tagName() == "file")
			child_name = Path::getName(filteredRelPath(child_key)) + ".txt";
		auto nearest_dir_key = nearestParentKeyOfType_recursor(child, Path::Type::Dir);
		auto nearest_dir_name = Path::getName(filteredRelPath(nearest_dir_key));
		auto stem_path_name = Path::getName(stemPathParent);
		QString next_stem_path;
		(stem_path_name == nearest_dir_name)
			? next_stem_path = stemPathParent
			: next_stem_path = stemPathParent / nearest_dir_name;
		auto short_path = next_stem_path / child_name;
		result << MetaDocRename{ child_key, short_path };
		result << getElementChildTree_recursor(child, next_stem_path);
		child = child.nextSiblingElement();
	}
	return result;
}

void Dom::handleRenamesList(QVector<MetaDocRename> renames)
{
	for (auto& rename : renames)
	{
		for (auto& item : renames_metaDoc)
		{
			if (item.key == rename.key)
				renames_metaDoc.removeAll(item);
		}
		auto& key = rename.key;
		auto& changed_rel_path = rename.changedRelPath;
		auto tar_rel_path = relPath(key);
		(tar_rel_path != nullptr)
			? renames_metaDoc << MetaDocRename{ key, changed_rel_path, tar_rel_path }
			: renames_metaDoc << MetaDocRename{ key, changed_rel_path };
	}
}

void Dom::makeMovePaths(QString& newPivotPath, QString& newPivotParentPath, QString pivotName, QDomElement fulcrumNode)
{
	auto fulcrum_parent_key = nearestParentKeyOfType_recursor(fulcrumNode, Path::Type::Dir);
	if (fulcrum_parent_key == nullptr)
	{
		newPivotParentPath = "story";
		newPivotPath = newPivotParentPath / pivotName;
	}
	else
	{
		auto fulcrum_parent_path = filteredRelPath(fulcrum_parent_key);
		newPivotPath = fulcrum_parent_path / pivotName;
		//
		newPivotParentPath = fulcrum_parent_path;
	}
}

// dom.cpp, fernanda