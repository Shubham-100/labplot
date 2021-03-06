/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2011 SCHUTZ Sacha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "QJsonModel.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QPalette>
#include <KLocalizedString>

QJsonTreeItem::QJsonTreeItem(QJsonTreeItem* parent) : mParent(parent) {}

QJsonTreeItem::~QJsonTreeItem() {
	qDeleteAll(mChilds);
}

void QJsonTreeItem::appendChild(QJsonTreeItem* item) {
	mChilds.append(item);
}

QJsonTreeItem* QJsonTreeItem::child(int row) {
	return mChilds.value(row);
}

QJsonTreeItem* QJsonTreeItem::parent() {
	return mParent;
}

int QJsonTreeItem::childCount() const {
	return mChilds.count();
}

int QJsonTreeItem::row() const {
	if (mParent)
		return mParent->mChilds.indexOf(const_cast<QJsonTreeItem*>(this));

	return 0;
}

void QJsonTreeItem::setKey(const QString& key) {
	mKey = key;
}

void QJsonTreeItem::setValue(const QString& value) {
	mValue = value;
}

void QJsonTreeItem::setType(const QJsonValue::Type type) {
	mType = type;
}

void QJsonTreeItem::setSize(int size) {
	mSize = size;
}

QString QJsonTreeItem::key() const {
	return mKey;
}

QString QJsonTreeItem::value() const {
	return mValue;
}

QJsonValue::Type QJsonTreeItem::type() const {
	return mType;
}

int QJsonTreeItem::size() const {
	return mSize;
}

QJsonTreeItem* QJsonTreeItem::load(const QJsonValue& value, QJsonTreeItem* parent) {
	auto* rootItem = new QJsonTreeItem(parent);
	rootItem->setKey("root");

	if (value.isObject())
		rootItem->setSize(QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact).size());
	else if (value.isArray())
		rootItem->setSize(QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact).size());

	if (value.isObject()) {
		//Get all QJsonValue childs
		for (QString key : value.toObject().keys()) {
			QJsonValue v = value.toObject().value(key);
			QJsonTreeItem* child = load(v,rootItem);
			child->setKey(key);
			child->setType(v.type());
			rootItem->appendChild(child);
		}
	} else if (value.isArray()) {
		//QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact).size();

		//Get all QJsonValue childs
		int index = 0;
		for (QJsonValue v : value.toArray()) {
			QJsonTreeItem* child = load(v,rootItem);
			child->setKey(QString::number(index));
			child->setType(v.type());
			rootItem->appendChild(child);
			++index;
		}
	} else {
		QString str = value.toVariant().toString();
		rootItem->setValue(str);
		rootItem->setType(value.type());
		rootItem->setSize(str.length());
	}

	return rootItem;
}

//=========================================================================

QJsonModel::QJsonModel(QObject* parent) : QAbstractItemModel(parent),
	mHeadItem(new QJsonTreeItem),
	mRootItem(new QJsonTreeItem(mHeadItem)) {

	mHeadItem->appendChild(mRootItem);
	mHeaders.append("Key");
	mHeaders.append("Value");
	mHeaders.append("Size in Bytes");
}

QJsonModel::~QJsonModel() {
	//delete mRootItem;
	delete mHeadItem;
}

void QJsonModel::clear() {
	beginResetModel();
	delete mHeadItem;
	mHeadItem = new QJsonTreeItem;
	mRootItem = new QJsonTreeItem(mHeadItem);
	mHeadItem->appendChild(mRootItem);
	endResetModel();
}

bool QJsonModel::load(const QString& fileName) {
	QFile file(fileName);
	bool success = false;
	if (file.open(QIODevice::ReadOnly)) {
		success = load(&file);
		file.close();
	} else
		success = false;

	return success;
}

bool QJsonModel::load(QIODevice* device) {
	return loadJson(device->readAll());
}

bool QJsonModel::loadJson(const QByteArray& json) {
	QJsonParseError error;
	QJsonDocument doc = QJsonDocument::fromJson(json, &error);
	if (error.error == QJsonParseError::NoError)
		return loadJson(doc);
	else {
		QMessageBox::critical(nullptr, i18n("Failed to load JSON document"),
							  i18n("Failed to load JSON document. Error: %1.", error.errorString()));
		return false;
	}

}

bool QJsonModel::loadJson(const QJsonDocument& jdoc) {
	if (!jdoc.isNull()) {
		beginResetModel();
		delete mHeadItem;

		mHeadItem = new QJsonTreeItem;

		if (jdoc.isArray()) {
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.array()), mHeadItem);
			mRootItem->setType(QJsonValue::Array);

		} else {
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.object()), mHeadItem);
			mRootItem->setType(QJsonValue::Object);
		}

		mHeadItem->appendChild(mRootItem);

		endResetModel();
		return true;
	}

	return false;
}

QVariant QJsonModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());

	if (role == Qt::DisplayRole) {
		if (index.column() == 0)
			return QString("%1").arg(item->key());

		else if (index.column() == 1)
			return QString("%1").arg(item->value());
		else {
			if (item->size() != 0)
				return QString("%1").arg(item->size());
			else
				return QString();
		}
	} else if (Qt::EditRole == role) {
		if (index.column() == 1)
			return QString("%1").arg(item->value());
	} else if (role == Qt::DecorationRole) {
		if (index.column() == 0) {
			QIcon icon;
			if (item->type() == QJsonValue::Array)
				icon = QIcon::fromTheme("labplot-json-array");
			else if (item->type() == QJsonValue::Object)
				icon = QIcon::fromTheme("labplot-json-object");

			if (qApp->palette().color(QPalette::Base).lightness() < 128) {
				//dark theme is used -> invert the icons which use black colors
				QImage image = icon.pixmap(64, 64).toImage(); //TODO: use different(standard?) pixel size?
				image.invertPixels();
				icon = QIcon(QPixmap::fromImage(image));
			}
			return icon;
		}
		return QIcon();
	}

	return QVariant();
}

bool QJsonModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (Qt::EditRole == role) {
		if (index.column() == 1) {
			auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
			item->setValue(value.toString());
			emit dataChanged(index, index, {Qt::EditRole});
			return true;
		}
	}

	return false;
}

QVariant QJsonModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal)
		return mHeaders.value(section);

	return QVariant();
}

QModelIndex QJsonModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex{};

	QJsonTreeItem* parentItem;

	if (!parent.isValid())
		parentItem = mHeadItem;
	else
		parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

	QJsonTreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);

	return QModelIndex{};
}

QModelIndex QJsonModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex{};

	auto* childItem = static_cast<QJsonTreeItem*>(index.internalPointer());
	QJsonTreeItem* parentItem = childItem->parent();

	if (parentItem == mHeadItem)
		return QModelIndex{};

	return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonModel::rowCount(const QModelIndex& parent) const {
	QJsonTreeItem* parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = mHeadItem;
	else
		parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int QJsonModel::columnCount(const QModelIndex& parent) const {
	Q_UNUSED(parent)
	return 3;
}

Qt::ItemFlags QJsonModel::flags(const QModelIndex& index) const {
	if (index.column() == 1)
		return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
	else
		return QAbstractItemModel::flags(index);
}

QJsonDocument QJsonModel::json() const {
	auto v = genJson(mRootItem);
	QJsonDocument doc;

	if (v.isObject())
		doc = QJsonDocument(v.toObject());
	else
		doc = QJsonDocument(v.toArray());

	return doc;
}

QJsonValue  QJsonModel::genJson(QJsonTreeItem* item) const {
	auto type = item->type();
	const int nchild = item->childCount();

	if (QJsonValue::Object == type) {
		QJsonObject jo;
		for (int i = 0; i < nchild; ++i) {
			auto ch = item->child(i);
			auto key = ch->key();
			jo.insert(key, genJson(ch));
		}
		return  jo;
	} else if (QJsonValue::Array == type) {
		QJsonArray arr;
		for (int i = 0; i < nchild; ++i) {
			auto ch = item->child(i);
			arr.append(genJson(ch));
		}
		return arr;
	} else {
		QJsonValue va(item->value());
		return va;
	}

}

QJsonDocument QJsonModel::genJsonByIndex(const QModelIndex& index) const {
	if (!index.isValid())
		return QJsonDocument();

	auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
	return QJsonDocument::fromVariant(genJson(item).toVariant());
}
