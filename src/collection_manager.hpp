#pragma once

#include "collection.hpp"
#include "picture_provider.hpp"

class CollectionManager : public QObject {
  Q_OBJECT
 public:
  enum class GroupingType : uint32_t {
    none = 0,
    directory = 1,
    day = 1 << 1,
    month = 1 << 2
  };
  Q_ENUM(GroupingType)

  enum class SortingType : uint32_t {
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

  Q_PROPERTY(PictureProvider* provider READ getProvider WRITE setProvider NOTIFY
                 providerChanged REQUIRED);
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
