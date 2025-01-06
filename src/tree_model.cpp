// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#include "tree_model.hpp"

#include <QDir>
#include <QFocusEvent>

#include <cstdint>
#include <memory>

DirectoryInfo::DirectoryInfo(QString path, QString name)
    : m_path(path), m_name(name), m_containsDirectories(false) {}

QString DirectoryInfo::getPath() const {
  return m_path;
}

void DirectoryInfo::setPath(QString path) {
  m_path = path;
}

QString DirectoryInfo::getName() const {
  return m_name;
}

void DirectoryInfo::setName(QString name) {
  if (m_name != name) {
    m_name = name;
  }
}

void DirectoryInfo::setContainsDirectories(bool flag) {
  if (m_containsDirectories != flag) {
    m_containsDirectories = flag;
  }
}

bool DirectoryInfo::getContainsDirectories() const {
  return m_containsDirectories;
}

DirectoryTreeModel::DirectoryTreeModel(QObject* parent) : m_root() {}

DirectoryTreeModel::~DirectoryTreeModel() {}

QVariant DirectoryTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  switch (role) {
    case Qt::DisplayRole: {
      const DirectoryNode* node =
          static_cast<const DirectoryNode*>(index.internalPointer());
      return node->data().getName();
    }
    case mayExpandFlag: {
      DirectoryNode* node =
          static_cast<DirectoryNode*>(index.internalPointer());
      QVariant qvar;
      qvar.setValue(node->data().getContainsDirectories());
      return qvar;
    }
    default:
      return {};
  }
}

Qt::ItemFlags DirectoryTreeModel::flags(const QModelIndex& index) const {
  return index.isValid() ? QAbstractItemModel::flags(index)
                         : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant DirectoryTreeModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? "Header"
                                                                  : QVariant{};
}

QModelIndex DirectoryTreeModel::index(int row,
                                      int column,
                                      const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }
  DirectoryNode* parentNode = getNodeFromModelIndex(parent);

  if (parentNode == nullptr) {
    return {};
  }

  if (row < parentNode->countChildNodes()) {
    return createIndex(row, column, parentNode->getChild(row));
  }
  return {};
}

QModelIndex DirectoryTreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return {};
  }
  DirectoryNode* node = static_cast<DirectoryNode*>(index.internalPointer());
  if (node->getParent() == m_root.get()) {
    return {};
  }
  return createIndex(node->getParent()->numChild(), 0, node->getParent());
}

int DirectoryTreeModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    if (m_root == nullptr) {
      return 0;
    } else {
      return m_root->countChildNodes();
    }
  } else {
    return static_cast<DirectoryNode*>(parent.internalPointer())
        ->countChildNodes();
  }
}

int DirectoryTreeModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

QHash<int, QByteArray> DirectoryTreeModel::roleNames() const {
  QHash<int, QByteArray> roleNames = QAbstractItemModel::roleNames();
  roleNames[AdditionalDataRole::mayExpandFlag] = "mayExpandFlag";
  return roleNames;
}

DirectoryTreeModel::DirectoryNode* DirectoryTreeModel::getNodeFromModelIndex(
    const QModelIndex& nodeIndex) const {
  DirectoryNode* node;
  if (!nodeIndex.isValid()) {
    node = m_root.get();
  } else {
    node = static_cast<DirectoryNode*>(nodeIndex.internalPointer());
  }
  return node;
}

QString DirectoryTreeModel::getTreeRoot() const {
  if (m_root.get() == nullptr) {
    return {};
  }
  return m_root->data().getPath();
}

void DirectoryTreeModel::setTreeRoot(QString rootDirectory) {
  if (!m_root || m_root->data().getPath() != rootDirectory) {
    beginResetModel();
    m_root.reset(
        new DirectoryNode(rootDirectory, QDir(rootDirectory).dirName()));
    endResetModel();
    loadChildren({});
    emit treeRootChanged();
    updateIsFilesystemRoot();
  }
}

bool DirectoryTreeModel::isFilesystemRoot() const {
  return QDir(m_root->data().getPath()).isRoot();
}

void DirectoryTreeModel::updateIsFilesystemRoot() {
  bool currentIsFilesystemRoot = isFilesystemRoot();
  if (currentIsFilesystemRoot != m_prevIsFilesystemRoot) {
    m_prevIsFilesystemRoot = currentIsFilesystemRoot;
    emit isFileSystemRootChanged();
  }
}

void DirectoryTreeModel::updateMayExpand(const QModelIndex& nodeIndex) {
  DirectoryNode* node = getNodeFromModelIndex(nodeIndex);
  QDir qdir(node->data().getPath());
  qdir.setFilter(QDir::Filter::Dirs | QDir::NoDotAndDotDot);
  const bool prevValue = node->data().getContainsDirectories();
  const bool newValue = qdir.count() > 0;
  if (newValue != prevValue) {
    node->data().setContainsDirectories(newValue);
    dataChanged(nodeIndex, nodeIndex, {mayExpandFlag});
  }
}

void DirectoryTreeModel::loadChildren(const QModelIndex& nodeIndex) {
  DirectoryNode* node = getNodeFromModelIndex(nodeIndex);
  QDir qdir(node->data().getPath());
  QString childPathPrefix = node->data().getPath();
  if (!childPathPrefix.endsWith('/')) {
    childPathPrefix += '/';
  }
  qdir.setFilter(QDir::Filter::Dirs | QDir::NoDotAndDotDot);
  const QStringList entries = qdir.entryList();
  beginInsertRows(nodeIndex, 0, entries.size() - 1);
  for (auto it : entries) {
    node->addChild(std::make_unique<DirectoryNode>(childPathPrefix + it, it));
  }
  endInsertRows();
  for (uint32_t i = 0; i < entries.size(); ++i) {
    updateMayExpand(index(i, 0, nodeIndex));
  }
}

void DirectoryTreeModel::updateTree(const QModelIndex& nodeIndex) {
  DirectoryNode* node = getNodeFromModelIndex(nodeIndex);
  updateMayExpand(nodeIndex);
  if (node->countChildNodes() == 0) {
    return;
  }
  QDir qdir(node->data().getPath());
  qdir.setFilter(QDir::Filter::Dirs | QDir::NoDotAndDotDot);
  QStringList entries = qdir.entryList();

  QString childPathPrefix = node->data().getPath();
  if (!childPathPrefix.endsWith('/')) {
    childPathPrefix += '/';
  }
  const uint32_t oldSize = uint32_t(node->countChildNodes());
  const uint32_t newSize = uint32_t(entries.size());
  const uint32_t maxUniqueEntries = oldSize + newSize;
  QList<uint32_t> order(maxUniqueEntries);
  QList<uint32_t> stayedNodes;
  QList<uint32_t> insertedNodes;
  uint32_t nDeleted = 0;
  uint32_t i = 0, j = 0;
  while (!(i == oldSize && j == newSize)) {
    if (i == oldSize) {
      const uint32_t pos = oldSize + insertedNodes.size();
      insertedNodes.append(j);
      order[j] = pos;
      ++j;
      continue;
    }
    if (j == newSize) {
      order[newSize + nDeleted] = i;
      ++i;
      ++nDeleted;
      continue;
    }
    QString nameFromOldEntries = node->getChild(i)->data().getName();
    if (nameFromOldEntries == entries[j]) {
      order[j] = i;
      stayedNodes.append(j);
      ++i;
      ++j;
    } else if (nameFromOldEntries > entries[j]) {
      const uint32_t pos = oldSize + insertedNodes.size();
      insertedNodes.append(j);
      order[j] = pos;
      ++j;
    } else {
      order[newSize + nDeleted] = i;
      ++i;
      ++nDeleted;
    }
  }
  if (insertedNodes.size() > 0) {
    beginInsertRows(nodeIndex, oldSize, oldSize + insertedNodes.size() - 1);
    for (uint32_t j : insertedNodes) {
      node->addChild(std::make_unique<DirectoryNode>(
          childPathPrefix + entries[j], entries[j]));
    }
    endInsertRows();
    for (uint32_t j = 0; j < insertedNodes.size(); ++j) {
      updateMayExpand(index(j + oldSize, 0, nodeIndex));
    }
  }
  if (nDeleted > 0 || insertedNodes.size() > 0) {
    order.resize(newSize + nDeleted);
    emit layoutAboutToBeChanged({nodeIndex});
    QModelIndexList beforeChange;
    for (uint32_t i = 0; i < node->countChildNodes(); ++i) {
      beforeChange.append(index(i, 0, nodeIndex));
    }
    node->reorderChildNodes(order);
    QModelIndexList afterChange;
    for (uint32_t i = 0; i < node->countChildNodes(); ++i) {
      afterChange.append(index(i, 0, nodeIndex));
    }
    changePersistentIndexList(beforeChange, afterChange);
    emit layoutChanged({nodeIndex});
  }
  if (nDeleted > 0) {
    beginRemoveRows(nodeIndex, newSize, newSize + nDeleted - 1);
    node->removeChildNodesFromEnd(nDeleted);
    endRemoveRows();
  }
  for (uint32_t j : stayedNodes) {
    updateTree(index(j, 0, nodeIndex));
  }
}

Q_INVOKABLE QString DirectoryTreeModel::getPath(const QModelIndex& nodeIndex) {
  DirectoryNode* node = getNodeFromModelIndex(nodeIndex);
  return node->data().getPath();
}

void DirectoryTreeModel::expandChildAtIndex(const QModelIndex& nodeIndex) {
  if (!nodeIndex.isValid()) {
    updateTree({});
  } else {
    DirectoryNode* node =
        static_cast<DirectoryNode*>(nodeIndex.internalPointer());
    if (node->countChildNodes() > 0) {
      updateTree(nodeIndex);
    } else {
      loadChildren(nodeIndex);
    }
  }
}

void DirectoryTreeModel::setUpperRoot() {
  QDir qdir = m_root->data().getPath();
  if (!qdir.cdUp()) {
    return;
  }
  beginResetModel();
  DirectoryNode::NodeUPtr prevRoot = std::move(m_root);
  m_root.reset(new DirectoryNode(qdir.absolutePath(), qdir.dirName()));
  endResetModel();
  beginInsertRows({}, 0, 0);
  m_root->addChild(std::move(prevRoot));
  endInsertRows();
  updateTree({});
  emit treeRootChanged();
  updateIsFilesystemRoot();
}

void DirectoryTreeModel::downRootTo(const QModelIndex& nodeIndex) {
  if (!nodeIndex.isValid()) {
    return;
  }
  DirectoryNode* node =
      static_cast<DirectoryNode*>(nodeIndex.internalPointer());
  DirectoryNode* parentNode = node->getParent();

  beginResetModel();
  std::vector<DirectoryNode::NodeUPtr> parentChildren =
      parentNode->detachChildNodes();
  m_root = std::move(parentChildren[node->numChild()]);
  endResetModel();

  if (m_root->countChildNodes() == 0) {
    loadChildren({});
  } else {
    updateTree({});
  }
  emit treeRootChanged();
  updateIsFilesystemRoot();
}
