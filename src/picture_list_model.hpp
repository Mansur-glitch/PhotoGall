// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#pragma once
#include "collection_manager.hpp"

#include <QAbstractListModel>

class PictureListModel : public QAbstractListModel {
  Q_OBJECT

 public:
  Q_DISABLE_COPY_MOVE(PictureListModel)

  enum class AdditionalDataRoles : int {
    groupFlag = Qt::UserRole + 1,
    filePath,
    fileName,
    directory,
    lastModified,
    size,
    resolution
  };

  PictureListModel(QObject* parent = nullptr);
  ~PictureListModel() override;

  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row,
                    int column,
                    const QModelIndex& parent = {}) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  QHash<int, QByteArray> roleNames() const override;

  CollectionManager* getCollectionManager() const;
  void setCollectionManager(CollectionManager* collectionManager);

  Q_PROPERTY(CollectionManager* collection READ getCollectionManager WRITE
                 setCollectionManager NOTIFY collectionManagerChanged REQUIRED);
 signals:
  void collectionManagerChanged();
 private slots:
  void beforeCollectionDataChange();
  void afterCollectionDataChanged();

 private:
  static constexpr quintptr groupBit = quintptr(1)
                                       << (8 * sizeof(quintptr) - 1);
  static constexpr int groupFlagDataRole = Qt::UserRole + 1;
  CollectionManager* m_collectionManager;
  Collection<PictureInfo>* m_collection;
  QList<quintptr> m_internalIndexList;
};
