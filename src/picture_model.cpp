#include "picture_model.hpp"
#include <cstdint>
#include <qdatetime.h>
#include <qfileinfo.h>

PictureCollection::PictureCollection(QObject* parent)
: QObject(parent), m_collection()
{}

Collection<PictureInfo>* PictureCollection::getCollection()
{
  return &m_collection;
}

PictureProvider::PictureProvider(QObject* parent)
: QObject(parent), m_directory(), m_lookupDepth(0)
{}

const QString& PictureProvider::getDirectory() const
{
  return m_directory;
}

void PictureCollection::picturesChanged(const PictureProvider::ChangedFiles changes)
{
  QList<PictureInfo> newItems;
  for (auto [filePath, changeType]: changes) {
    if (changeType == PictureProvider::FileChangeType::removed) {
      m_collection.removeItemByInnerIndex();
    }
    QFileInfo fileInfo(filePath);
    PictureInfo p;
    p.date = fileInfo.fileTime(QFile::FileTime::FileModificationTime).date();
    p.directory = fileInfo.path();
    p.name = fileInfo.fileName();
    p.size = fileInfo.size();
    newItems.append(p);
  }
  m_collection.addItems(newItems);
}

void PictureProvider::setAllRemoved()
{
  for (auto& value: m_files) {
    value.type = FileChangeType::removed;
  }
}
  
const QList<QPair<QString, PictureProvider::FileChangeType>> PictureProvider::getChanges() const
{
  QList<QPair<QString, FileChangeType>> changes;

  for (const auto [filePath, changeInfo]:  m_files.asKeyValueRange()) {
    if (changeInfo.type == FileChangeType::notChanged) {
      continue;
    }
    changes.append(QPair<QString, FileChangeType>{filePath, changeInfo.type});
  }
  return changes;
}

void PictureProvider::removeMarked()
{
  for (auto it = m_files.begin(); it != m_files.end(); ) {
    const FileChangeType changeType = it->type;
    if (changeType == FileChangeType::removed) {
      it = m_files.erase(it);
    } else {
      ++it;
    }
  }
}

void PictureProvider::updateFiles()
{
  QDir qdir(m_directory);
  int currentDepth = 0;
  QList<QFileInfoList> infoList(m_lookupDepth);
  QList<uint32_t> entryNum(m_lookupDepth);
  infoList[0] = qdir.entryInfoList();
  entryNum[0] = 0;
  do {
    for (;entryNum[currentDepth] < infoList[currentDepth].size(); ++entryNum[currentDepth]) {
      const QFileInfo& fileInfo = infoList[currentDepth][entryNum[currentDepth]]; 
      if (fileInfo.isDir() && currentDepth < m_lookupDepth) {
        ++currentDepth;
        infoList[currentDepth] = QDir(fileInfo.absoluteFilePath()).entryInfoList();
        entryNum[currentDepth] = 0;
        continue;
      }
      checkFileChanges(fileInfo);
    }
    --currentDepth;
  } while (currentDepth != -1);
}

void PictureProvider::checkFileChanges(const QFileInfo& file)
{
  QString filePath = file.absoluteFilePath();
  QDateTime modificationTime = file.fileTime(QFile::FileTime::FileModificationTime);
  if (! m_files.contains(filePath)) {
    m_files.insert(filePath, FileChangeInfo{FileChangeType::added, modificationTime});
  } else if (m_files[filePath].modificationTime != modificationTime) {
    m_files[filePath].modificationTime = modificationTime;
    m_files[filePath].type = FileChangeType::changed;
  } else {
    m_files[filePath].type = FileChangeType::notChanged;
  }
}
  

void PictureProvider::setDirectory(const QString& directory)
{
  if (m_directory != directory) {
    setAllRemoved();
    m_directory = directory;
    updateFiles();
    emit directoryChanged();
    emit picturesChanged(getChanges());
    removeMarked();
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
