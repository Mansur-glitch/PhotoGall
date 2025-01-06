// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#pragma once

#include <QAbstractItemModel>
#include <QDate>
#include <QDir>
#include <QObject>

struct PictureInfo {
  QString filePath;
  QString name;
  QString directory;
  QDateTime datetime{};
  size_t size{0};
};

class PictureProvider : public QObject {
  Q_OBJECT
 public:
  enum class FileChangeType { notChanged = 0, added, removed, changed };
  using ChangedFiles = QList<QPair<QString, FileChangeType>>;

  struct FileChangeInfo {
    FileChangeType type;
    QDateTime modificationTime;
  };

  PictureProvider(QObject* parent = nullptr);
  const QString& getDirectory() const;
  void setDirectory(const QString& directory);
  void updateFiles();
  void setLookupDepth(int depth);
  int getLookupDepth() const noexcept;
  Q_PROPERTY(QString directory READ getDirectory WRITE setDirectory NOTIFY
                 directoryChanged);
  Q_PROPERTY(int lookupDepth READ getLookupDepth WRITE setLookupDepth NOTIFY
                 lookupDepthChanged);
 signals:
  void directoryChanged();
  void lookupDepthChanged();
  void picturesChanged(const ChangedFiles changes);

 private:
  void markAllRemoved();
  void removeMarked();
  void determineChanges(const QFileInfo& file);
  const ChangedFiles getChanges() const;
  static bool isAppropriateFileExtension(const QFileInfo& fileInfo);

  int m_lookupDepth{0};
  QString m_directory;
  QHash<QString, FileChangeInfo> m_filesUpdates;
};
