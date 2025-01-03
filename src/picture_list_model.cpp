#include "picture_list_model.hpp"

#include <QImage>

PictureListModel::PictureListModel(QObject* parent)
    : QAbstractListModel(parent),
      m_collection(nullptr),
      m_collectionManager(nullptr),
      m_internalIndexList() {}

PictureListModel::~PictureListModel() {}

QVariant PictureListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  quintptr id = index.internalId();
  bool isGroup = id & groupBit;
  switch (role) {
    case Qt::DisplayRole: {
      if (isGroup) {
        return m_collectionManager->getGroupTextInfo(
            m_collection->getGroupByOrder(id ^ groupBit));
      } else {
        return m_collection->getItem(id).name;
      }
    }
    case int(AdditionalDataRoles::groupFlag):
      return isGroup;
  }
  if (isGroup) {
    return {};
  }
  const PictureInfo& pinfo = m_collection->getItem(id);
  switch (AdditionalDataRoles(role)) {
    case AdditionalDataRoles::filePath:
      return pinfo.filePath;
    case AdditionalDataRoles::fileName:
      return pinfo.name;
    case AdditionalDataRoles::directory:
      return pinfo.directory;
    case AdditionalDataRoles::lastModified:
      return pinfo.datetime.toString("dd.MM.yyyy hh:mm:ss");
    case AdditionalDataRoles::size:
      return QString::number((double)pinfo.size / 1024 / 1024, 'g', 2) + " MB";
    case AdditionalDataRoles::resolution: {
      const QImage img(pinfo.filePath);
      return QString::number(img.width()) + "x" + QString::number(img.height());
    }
    default:
      break;
  }
  return {};
}

Qt::ItemFlags PictureListModel::flags(const QModelIndex& index) const {
  return index.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
}

QVariant PictureListModel::headerData(int section,
                                      Qt::Orientation orientation,
                                      int role) const {
  return orientation == Qt::Horizontal && role == Qt::DisplayRole
             ? "Heder provided"
             : QVariant{};
}

QModelIndex PictureListModel::index(int row,
                                    int column,
                                    const QModelIndex& parent) const {
  if (parent.isValid()) {
    return {};
  } else {
    return createIndex(row, 0, m_internalIndexList[row]);
  }
}

QModelIndex PictureListModel::parent(const QModelIndex& index) const {
  return {};
}

int PictureListModel::rowCount(const QModelIndex& parent) const {
  // Have no root
  if (parent.isValid()) {
    return 0;
  } else {
    return m_internalIndexList.size();
  }
}

QHash<int, QByteArray> PictureListModel::roleNames() const {
  QHash<int, QByteArray> roleNames = QAbstractItemModel::roleNames();
  roleNames[int(AdditionalDataRoles::groupFlag)] = "groupFlag";
  roleNames[int(AdditionalDataRoles::filePath)] = "filePath";
  roleNames[int(AdditionalDataRoles::fileName)] = "fileName";
  roleNames[int(AdditionalDataRoles::directory)] = "directory";
  roleNames[int(AdditionalDataRoles::lastModified)] = "lastModified";
  roleNames[int(AdditionalDataRoles::size)] = "size";
  roleNames[int(AdditionalDataRoles::resolution)] = "resolution";
  return roleNames;
}

CollectionManager* PictureListModel::getCollectionManager() const {
  return m_collectionManager;
}

void PictureListModel::setCollectionManager(
    CollectionManager* collectionManager) {
  if (m_collectionManager != collectionManager) {
    m_collectionManager = collectionManager;
    m_collection = collectionManager->getCollection();
    emit collectionManagerChanged();
    QObject::connect(collectionManager,
                     &CollectionManager::beforeCollectionDataChange, this,
                     &PictureListModel::beforeCollectionDataChange);
    QObject::connect(collectionManager,
                     &CollectionManager::afterCollectionDataChanged, this,
                     &PictureListModel::afterCollectionDataChanged);
  }
}

void PictureListModel::beforeCollectionDataChange() {
  beginResetModel();
}

void PictureListModel::afterCollectionDataChanged() {
  m_internalIndexList.clear();
  if (m_collection->filteredSize() != 0) {
    for (quintptr groupNum = 0; groupNum < m_collection->countGroups();
         ++groupNum) {
      Collection<PictureInfo>::DisplayGroup g =
          m_collection->getGroupByOrder(groupNum);
      m_internalIndexList.append(groupNum | groupBit);
      for (quintptr j = 0; j < g.size(); ++j) {
        m_internalIndexList.append(g.begin() + j);
      }
    }
  }
  endResetModel();
}
