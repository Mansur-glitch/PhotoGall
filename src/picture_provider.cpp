#include "picture_provider.hpp"
#include "collection.hpp"

#include <QImage>
#include <cstdint>
#include <qregularexpression.h>

CollectionManager::CollectionManager(QObject* parent)
: QObject(parent), m_collection(), m_fileIndexes(), m_provider(nullptr),
  m_groupingType(CollectionManager::GroupingType::none),
  m_sortingType(CollectionManager::SortingType::name), m_fInverseSorting(false)
{}

Collection<PictureInfo>* CollectionManager::getCollection()
{
  return &m_collection;
}

PictureProvider* CollectionManager::getProvider() const
{
  return m_provider;
}

void CollectionManager::setProvider(PictureProvider* provider)
{
  if (m_provider != provider) {
    m_provider = provider;
    QObject::connect(provider, &PictureProvider::picturesChanged, this, &CollectionManager::picturesChanged);
    emit providerChanged();
  }
}

PictureProvider::PictureProvider(QObject* parent)
: QObject(parent), m_directory(), m_lookupDepth(0)
{}

const QString& PictureProvider::getDirectory() const
{
  return m_directory;
}

void CollectionManager::picturesChanged(const PictureProvider::ChangedFiles changes)
{
  emit beforeCollectionDataChange();
  QList<PictureInfo> newItems;
  for (auto [filePath, changeType]: changes) {
    if (changeType == PictureProvider::FileChangeType::removed) {
      auto it = m_fileIndexes.find(filePath);
      if (it != m_fileIndexes.end()) {
        m_collection.removeItemByInnerIndex(*it);
        m_fileIndexes.erase(it);
      }
    } else if (changeType == PictureProvider::FileChangeType::added) {
      QFileInfo fileInfo(filePath);
      PictureInfo p;
      p.filePath = filePath;
      p.datetime = fileInfo.fileTime(QFile::FileTime::FileModificationTime);
      p.directory = fileInfo.path();
      p.name = fileInfo.fileName();
      p.size = fileInfo.size();
      // {
      //   const QImage img(filePath);
      //   p.resolution = {img.width(), img.height()};
      // }
      newItems.append(p);
      m_fileIndexes.insert(filePath, -1);
    }
  }
  m_collection.addItems(newItems);
  // Implicitly update collection
  const volatile uint32_t innerSize = m_collection.innerSize();
  const QList<PictureInfo>& data = m_collection.getData();
  // const QBitArray& finalFilter = m_collection.getVisibleMask();
  uint32_t fileIndex = 0;

  for (uint32_t i = 0; i < data.size(); ++i) {
    auto it = m_fileIndexes.find(data[i].filePath);
    if (it != m_fileIndexes.end()) {
      *it = fileIndex++;     
    }
  }
  emit afterCollectionDataChanged();
}

QString CollectionManager::getGroupTextInfo(const Collection<PictureInfo>::DisplayGroup& group)
{
  const PictureInfo& pinfo = m_collection.getItem(group.begin());
  if (m_groupingType == GroupingType::none) {
    return pinfo.directory;
  }

  QString text;
  bool needNewLine = false;
  if ((uint32_t)m_groupingType & (uint32_t)GroupingType::directory) {
    text += pinfo.directory;
    needNewLine = true;
  }
  if ((uint32_t)m_groupingType & (uint32_t)GroupingType::day) {
    if (needNewLine) {
      text += '\n';
    }
    text +=  tr("Day: ") + pinfo.datetime.date().toString();
    needNewLine = true;
  }
  if ((uint32_t)m_groupingType & (uint32_t)GroupingType::month) {
    if (needNewLine) {
      text += '\n';
    }
    text += tr("Month: ") + QString::number(pinfo.datetime.date().month());
  }
  return text;
}

void CollectionManager::setGrouping(GroupingType type, bool activate)
{
  emit beforeCollectionDataChange();
  m_groupingType = GroupingType(uint32_t(m_groupingType) ^ uint32_t(type));
  if (activate) {
    switch(type) {
      case GroupingType::directory:
        m_collection.setGrouping(0,
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.directory == rhs.directory;
                                  },
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.directory < rhs.directory;
                                  });
        break;
      case GroupingType::day:
        m_collection.setGrouping(1,
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.datetime.date() == rhs.datetime.date();
                                  },
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.datetime < rhs.datetime;
                                  });
        break;
      case GroupingType::month:
        m_collection.setGrouping(2,
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.datetime.date().year() == rhs.datetime.date().year()
                                            && lhs.datetime.date().month() == rhs.datetime.date().month();
                                  },
                                  [](const PictureInfo& lhs, const PictureInfo& rhs)
                                  {
                                    return lhs.datetime < rhs.datetime;
                                  });
        break;
      default:
        break;
    }
  } else {
    uint32_t typeNum = 0;
    switch (type) {
      case GroupingType::directory:
      typeNum = 0;
      break;
      case GroupingType::day:
      typeNum = 1;
      break;
      case GroupingType::month:
      typeNum = 2;
      break;
      default:
        break;
    }
    m_collection.resetGrouping(typeNum);
  }
  emit afterCollectionDataChanged();
}

void CollectionManager::setSorting(SortingType type)
{
  emit beforeCollectionDataChange();
  m_sortingType = type;
  const bool fInverse = m_fInverseSorting; 

  switch (type) {
    case SortingType::name:
      m_collection.setSorting([fInverse](const PictureInfo& lhs, const PictureInfo& rhs)
                              {
                                return fInverse ^ lhs.name < rhs.name;
                              });
      break;
    case SortingType::date:
      m_collection.setSorting([fInverse](const PictureInfo& lhs, const PictureInfo& rhs)
                              {
                                return fInverse ^ lhs.datetime < rhs.datetime;
                              });
      break;
    case SortingType::size:
      m_collection.setSorting([fInverse](const PictureInfo& lhs, const PictureInfo& rhs)
                              {
                                return fInverse ^ (lhs.size < rhs.size);
                              });
      break;
  }
  emit afterCollectionDataChanged();
}

void CollectionManager::setSortingInverseFlag(bool flag)
{
  m_fInverseSorting = flag;  
  setSorting(m_sortingType);
}

void CollectionManager::setFilteringByDate(QDate date, bool dateSince_dateUntil)
{
  emit beforeCollectionDataChange();
  uint32_t filterNum = dateSince_dateUntil ? 1: 0;
  if (dateSince_dateUntil) {
    m_collection.setFiltering(filterNum, [date](const PictureInfo& item)
                              {
                                return item.datetime.date() <= date;
                              });
  } else {
    m_collection.setFiltering(filterNum, [date](const PictureInfo& item)
                              {
                                return item.datetime.date() >= date;
                              });
  }
  emit afterCollectionDataChanged();
}

void CollectionManager::resetFilteringByDate(bool dateSince_dateUntil)
{
  emit beforeCollectionDataChange();
  uint32_t filterNum = dateSince_dateUntil ? 1: 0;
  m_collection.resetFiltering(filterNum);
  emit afterCollectionDataChanged();
}

void CollectionManager::setSearchPattern(QString pattern)
{
  emit beforeCollectionDataChange();
  if (pattern.isEmpty()) {
    m_collection.resetFiltering(2);
  } else {
    QRegularExpression rgexpr(pattern, QRegularExpression::CaseInsensitiveOption);
    m_collection.setFiltering(2, [rgexpr](const PictureInfo& item)
  {
    QRegularExpressionMatch match = rgexpr.match(item.name);
    return match.hasMatch();
  });
  }
  emit afterCollectionDataChanged();
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
  QDir qdir {m_directory};
  qdir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
  int currentDepth = 0;
  QList<QFileInfoList> infoList(m_lookupDepth + 1);
  QList<uint32_t> entryNum(m_lookupDepth + 1);
  infoList[0] = qdir.entryInfoList();
  entryNum[0] = 0;
  do {
    for (;entryNum[currentDepth] < infoList[currentDepth].size();) {
      const QFileInfo& fileInfo = infoList[currentDepth][entryNum[currentDepth]]; 
       ++entryNum[currentDepth];
      if (fileInfo.isDir() && currentDepth < m_lookupDepth) {
        ++currentDepth;
        QDir childDir = QDir(fileInfo.absoluteFilePath());
        childDir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
        infoList[currentDepth] = childDir.entryInfoList();
        entryNum[currentDepth] = 0;
        continue;
      } else if (isAppropriateFileExtension(fileInfo)) {
        checkFileChanges(fileInfo);
      }
    }
    --currentDepth;
  } while (currentDepth != -1);
}

bool PictureProvider::isAppropriateFileExtension(const QFileInfo& fileInfo)
{
  static const QHash<QString, uint32_t> appropriateExtensions {{"png", 0}, {"jpg", 1}, {"jpeg", 2}};
  return appropriateExtensions.contains(fileInfo.suffix());
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
  if (m_directory != directory && QDir(directory).exists()) {
    setAllRemoved();
    m_directory = directory;
    updateFiles();
    emit directoryChanged();
    emit picturesChanged(getChanges());
    removeMarked();
  }
}

void PictureProvider::setLookupDepth(int depth)
{
  if (m_lookupDepth != depth) {
    m_lookupDepth = depth;
    emit lookupDepthChanged();

    setAllRemoved();
    updateFiles();
    emit picturesChanged(getChanges());
    removeMarked();
  }
}

int PictureProvider::getLookupDepth() const noexcept
{
  return m_lookupDepth;
}
