// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#pragma once

#include <boost/algorithm/apply_permutation.hpp>

#include <QAbstractItemModel>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickItem>
#include <QValidator>

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

template <class T>
class TreeNode {
 public:
  using NodeUPtr = std::unique_ptr<TreeNode<T>>;

  template <class... Args>
  TreeNode(Args&&... args)
      : m_data(std::forward<Args>(args)...),
        m_parent(nullptr),
        m_nChildNum(0),
        m_children() {}

  void addChild(NodeUPtr node) {
    m_children.emplace_back(std::move(node));
    NodeUPtr& addedNode = m_children[m_children.size() - 1];
    addedNode->m_parent = this;
    addedNode->m_nChildNum = m_children.size() - 1;
  }

  void removeChildNodesFromEnd(uint32_t count) {
    m_children.resize(m_children.size() - count);
  }

  void reorderChildNodes(QList<uint32_t> newOrder) {
    boost::algorithm::apply_permutation(m_children, newOrder);
    uint32_t i = 0;
    for (NodeUPtr& p : m_children) {
      p->m_nChildNum = i++;
    }
  }

  T& data() { return m_data; }

  const T& data() const { return m_data; }

  TreeNode* getChild(int num) const { return m_children[num].get(); }

  TreeNode* getLastChild() const {
    return m_children[m_children.size() - 1].get();
  }

  TreeNode* getParent() const noexcept { return m_parent; }

  uint32_t numChild() const noexcept { return m_nChildNum; }

  uint32_t countChildNodes() const noexcept { return m_children.size(); }

  std::vector<NodeUPtr> detachChildNodes() {
    std::vector<NodeUPtr> result = std::move(m_children);
    m_children.clear();
    return result;
  }

 private:
  T m_data;
  TreeNode* m_parent;
  uint32_t m_nChildNum;
  std::vector<NodeUPtr> m_children;
};

class DirectoryInfo {
 public:
  DirectoryInfo(QString path, QString name);
  QString getPath() const;
  void setPath(QString path);
  QString getName() const;
  void setName(QString name);
  bool getContainsDirectories() const;
  void setContainsDirectories(bool flag);

 private:
  QString m_path;
  QString m_name;
  bool m_containsDirectories;
};

class DirectoryTreeModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  using DirectoryNode = TreeNode<DirectoryInfo>;
  Q_DISABLE_COPY_MOVE(DirectoryTreeModel)

  enum AdditionalDataRole { mayExpandFlag = Qt::UserRole + 1 };

  explicit DirectoryTreeModel(QObject* parent = nullptr);
  ~DirectoryTreeModel() override;

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setTreeRoot(QString rootDirectory);
  QString getTreeRoot() const;
  bool isFilesystemRoot() const;
  void updateIsFilesystemRoot();

  Q_INVOKABLE QString getPath(const QModelIndex& nodeIndex);
  Q_INVOKABLE void expandChildAtIndex(const QModelIndex& index);
  Q_INVOKABLE void setUpperRoot();
  Q_INVOKABLE void downRootTo(const QModelIndex& index);

  Q_PROPERTY(QString rootDirectory READ getTreeRoot WRITE setTreeRoot NOTIFY
                 treeRootChanged);
  Q_PROPERTY(bool isFilesystemRoot READ isFilesystemRoot NOTIFY
                 isFileSystemRootChanged);
 signals:
  void treeRootChanged();
  void isFileSystemRootChanged();

 private:
  DirectoryNode* getNodeFromModelIndex(const QModelIndex& nodeIndex) const;
  void loadChildren(const QModelIndex& nodeIndex);
  void updateMayExpand(const QModelIndex& nodeIndex);
  void updateTree(const QModelIndex& nodeIndex);

  DirectoryNode::NodeUPtr m_root;
  bool m_prevIsFilesystemRoot;
};
