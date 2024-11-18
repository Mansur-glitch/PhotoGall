#pragma once
#include "picture_collection.hpp"

#include <QObject>
#include <QDate>
#include <QDir>
#include <QAbstractItemModel>
#include <cstdint>
#include <qcontainerfwd.h>
#include <qdatetime.h>
#include <qfileinfo.h>

struct PictureInfo
{
  QString name;
  QString directory;
  QDate date {};
  size_t size {0};
  struct Resolution
  {
    size_t x {0}, y {0};
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
  Q_PROPERTY(QString directory READ getDirectory WRITE setDirectory NOTIFY directoryChanged);
  void updateFiles();
signals:
  void directoryChanged();
  void picturesChanged(const ChangedFiles changes);
private: 
  void setAllRemoved();
  void removeMarked();
  void checkFileChanges(const QFileInfo& file);
  const ChangedFiles getChanges() const;

  uint32_t m_lookupDepth {0};
  QString m_directory;
  QHash<QString, FileChangeInfo> m_files;
};

class PictureCollection: public QObject
{
  Q_OBJECT
public:
  Collection<PictureInfo>* getCollection();
private:
  Collection<PictureInfo> m_collection;
private slots:
  void picturesChanged(const PictureProvider::ChangedFiles changes);
};


class GroupedPictureModel: public QAbstractItemModel
{
  Q_OBJECT

public:
  Q_DISABLE_COPY_MOVE(GroupedPictureModel)

  GroupedPictureModel(QObject* parent = nullptr);
  ~GroupedPictureModel() override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const  QModelIndex& parent = {}) const override;
  QModelIndex parent(const  QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;

  PictureCollection* getCollection() const;
  void setCollection(PictureCollection* collection);

  Q_PROPERTY(PictureCollection* collection READ getCollection WRITE setCollection NOTIFY collectionChanged REQUIRED);
signals:
  void collectionChanged();

private:
  bool isIndexValid(const QModelIndex& index) const;
  PictureCollection* m_collectionProperty;
  Collection<PictureInfo>* m_collection;
  static constexpr quintptr groupBit = quintptr(1) << (8 * sizeof(quintptr) - 1);
};
