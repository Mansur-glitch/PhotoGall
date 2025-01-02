#pragma once
#include "collection.hpp"

#include <QObject>
#include <QDate>
#include <QDir>
#include <QAbstractItemModel>
#include <cstdint>

struct PictureInfo
{
  QString filePath;
  QString name;
  QString directory;
  QDateTime datetime {};
  size_t size {0};
  // UNUSED
  struct Resolution
  {
    int x {0}, y {0};
    bool operator<(const Resolution& other) {
      return x * y < other.x * other.y;
    }
  } resolution {0, 0};
};

class PictureProvider: public QObject
{
  Q_OBJECT
public:
  enum class FileChangeType
  {
    notChanged = 0,
    added,
    removed,
    changed
  };
  using ChangedFiles = QList<QPair<QString, FileChangeType>>;

  struct FileChangeInfo
  {
    FileChangeType type;    
    QDateTime modificationTime;
  };

  PictureProvider(QObject* parent = nullptr);
  const QString& getDirectory() const;
  void setDirectory(const QString& directory);
  void updateFiles();
  void setLookupDepth(int depth);
  int getLookupDepth() const noexcept;
  Q_PROPERTY(QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged);
  Q_PROPERTY(int lookupDepth READ getLookupDepth WRITE setLookupDepth NOTIFY lookupDepthChanged);
signals:
  void directoryChanged();
  void lookupDepthChanged();
  void picturesChanged(const ChangedFiles changes);
private: 
  void setAllRemoved();
  void removeMarked();
  void checkFileChanges(const QFileInfo& file);
  const ChangedFiles getChanges() const;
  static bool isAppropriateFileExtension(const QFileInfo& fileInfo);

  int m_lookupDepth {0};
  QString m_directory;
  QHash<QString, FileChangeInfo> m_files;
};

class CollectionManager: public QObject
{
  Q_OBJECT
public:
  enum class GroupingType: uint32_t
  {
    none = 0,
    directory = 1,
    day = 1 << 1,
    month = 1 << 2
  };
  Q_ENUM(GroupingType)

  enum class SortingType: uint32_t
  {
    name,
    date,
    size,
  };
  Q_ENUM(SortingType)

  CollectionManager(QObject* parent = nullptr);
  Collection<PictureInfo>* getCollection();
  PictureProvider* getProvider() const;
  void setProvider(PictureProvider*);
  QString getGroupTextInfo(const Collection<PictureInfo>::DisplayGroup& group);
  Q_INVOKABLE void setGrouping(GroupingType type, bool activate);
  Q_INVOKABLE void setSorting(SortingType type);
  Q_INVOKABLE void setSortingInverseFlag(bool flag);
  Q_INVOKABLE void setFilteringByDate(QDate date, bool dateSince_dateUntil);
  Q_INVOKABLE void resetFilteringByDate(bool dateSince_dateUntil);
  Q_INVOKABLE void setSearchPattern(QString pattern);

  Q_PROPERTY(PictureProvider* provider READ getProvider WRITE setProvider NOTIFY providerChanged REQUIRED);
signals:
  void providerChanged(); 
  void beforeCollectionDataChange();
  void afterCollectionDataChanged();

private:
  Collection<PictureInfo> m_collection;
  QHash<QString, int> m_fileIndexes;
  PictureProvider* m_provider;
  GroupingType m_groupingType;
  SortingType m_sortingType;
  bool m_fInverseSorting;

private slots:
  void picturesChanged(const PictureProvider::ChangedFiles changes);
};
