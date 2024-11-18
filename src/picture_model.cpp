#include "picture_model.hpp"

Collection<PictureInfo>* PictureCollection::getCollection()
{
  return &m_collection;
}

PictureProvider::PictureProvider(QObject* parent)
: QObject(parent)
{}

const QString& PictureProvider::getDirectory() const
{
  return m_directory;
}

void PictureProvider::updateCollection()
{
  m_collection->clearData();
  QDir qdir(m_directory);
  qdir.setFilter(QDir::Files);
  QFileInfoList infoList = qdir.entryInfoList();

  QList<PictureInfo> newItems;
  for (const QFileInfo& fileInfo: infoList) {
    PictureInfo p;
    p.date = fileInfo.fileTime(QFile::FileTime::FileModificationTime).date();
    p.directory = fileInfo.path();
    p.name = fileInfo.fileName();
    p.size = fileInfo.size();
    newItems.append(p);
  }
  m_collection->addItems(newItems);
}

void PictureProvider::setDirectory(const QString& directory)
{
  if (m_directory != directory) {
    m_directory = directory;
    emit directoryChanged();
    if (m_collection != nullptr) {
      updateCollection();
    }
  }
}

PictureCollection* PictureProvider::getCollection() const
{
  return m_collectionProperty;
}

void PictureProvider::setCollection(PictureCollection* collection)
{
  if (m_collectionProperty != collection) {
    m_collectionProperty = collection;
    m_collection = collection->getCollection();
    if (! m_directory.isEmpty()) {
      updateCollection();
    }
  }
}

GroupedPictureModel::GroupedPictureModel(QObject* parent)
: QAbstractItemModel(parent), m_collection(nullptr),
  m_collectionProperty(nullptr)
{}

GroupedPictureModel::~GroupedPictureModel()
{}

bool GroupedPictureModel::isIndexValid(const QModelIndex& index) const
{
  return index.isValid() && index.internalId() < m_collection->filteredSize();
}

QVariant GroupedPictureModel::data(const QModelIndex &index, int role) const
{
  bool fValidQuery = isIndexValid(index) && role == Qt::ItemDataRole::DisplayRole;
  if (! fValidQuery) {
    return {};
  }
  quintptr id =  index.internalId();
  const PictureInfo& item = m_collection->getItem(id);
  return item.name;
  // if (m_grList.isItem(id)) {
  //   return m_grList.getItem(id).get()->name;
  // } else {
  //   return m_grList.getGroup(id).get()->m_id;
  // }
}

Qt::ItemFlags GroupedPictureModel::flags(const QModelIndex& index) const
{
  return isIndexValid(index) ? Qt::ItemIsEnabled : Qt::NoItemFlags; 
}


QVariant GroupedPictureModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ?
  "Heder provided" : QVariant{};
}

QModelIndex GroupedPictureModel::index(int row, int column, const  QModelIndex& parent) const
{
  // Have no root
  if (parent.isValid()) {
    return {};
  } else {
    return createIndex(row, 0, row);
  }
}

QModelIndex GroupedPictureModel::parent(const QModelIndex& index) const
{
  return {};
}

int GroupedPictureModel::rowCount(const QModelIndex& parent) const
{
  // Have no root
  if (parent.isValid()) {
    return 0;
  } else {
    return m_collection->filteredSize();
  }
}

int GroupedPictureModel::columnCount(const QModelIndex& parent) const
{
  return 1;
}

PictureCollection* GroupedPictureModel::getCollection() const
{
  return m_collectionProperty;
}

void GroupedPictureModel::setCollection(PictureCollection* collection)
{
  if (m_collectionProperty != collection) {
    m_collectionProperty = collection;
    m_collection = collection->getCollection();
    emit collectionChanged();
  }
}
