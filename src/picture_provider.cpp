// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#include "picture_provider.hpp"

#include <cstdint>

PictureProvider::PictureProvider(QObject* parent)
    : QObject(parent), m_directory(), m_lookupDepth(0) {}

const QString& PictureProvider::getDirectory() const {
  return m_directory;
}

void PictureProvider::markAllRemoved() {
  for (auto& value : m_filesUpdates) {
    value.type = FileChangeType::removed;
  }
}

const QList<QPair<QString, PictureProvider::FileChangeType>>
PictureProvider::getChanges() const {
  QList<QPair<QString, FileChangeType>> changes;

  for (const auto [filePath, changeInfo] : m_filesUpdates.asKeyValueRange()) {
    if (changeInfo.type == FileChangeType::notChanged) {
      continue;
    }
    changes.append(QPair<QString, FileChangeType>{filePath, changeInfo.type});
  }
  return changes;
}

void PictureProvider::removeMarked() {
  for (auto it = m_filesUpdates.begin(); it != m_filesUpdates.end();) {
    const FileChangeType changeType = it->type;
    if (changeType == FileChangeType::removed) {
      it = m_filesUpdates.erase(it);
    } else {
      ++it;
    }
  }
}

void PictureProvider::updateFiles() {
  QDir qdir{m_directory};
  qdir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs |
                 QDir::Filter::NoDotAndDotDot);
  int currentDepth = 0;
  QList<QFileInfoList> infoList(m_lookupDepth + 1);
  QList<uint32_t> entryNum(m_lookupDepth + 1);
  infoList[0] = qdir.entryInfoList();
  entryNum[0] = 0;
  do {
    for (; entryNum[currentDepth] < infoList[currentDepth].size();) {
      const QFileInfo& fileInfo =
          infoList[currentDepth][entryNum[currentDepth]];
      ++entryNum[currentDepth];
      if (fileInfo.isDir() && currentDepth < m_lookupDepth) {
        ++currentDepth;
        QDir childDir = QDir(fileInfo.absoluteFilePath());
        childDir.setFilter(QDir::Filter::Files | QDir::Filter::Dirs |
                           QDir::Filter::NoDotAndDotDot);
        infoList[currentDepth] = childDir.entryInfoList();
        entryNum[currentDepth] = 0;
        continue;
      } else if (isAppropriateFileExtension(fileInfo)) {
        determineChanges(fileInfo);
      }
    }
    --currentDepth;
  } while (currentDepth != -1);
}

bool PictureProvider::isAppropriateFileExtension(const QFileInfo& fileInfo) {
  static const QHash<QString, uint32_t> appropriateExtensions{
      {"png", 0}, {"jpg", 1}, {"jpeg", 2}};
  return appropriateExtensions.contains(fileInfo.suffix());
}

void PictureProvider::determineChanges(const QFileInfo& file) {
  QString filePath = file.absoluteFilePath();
  QDateTime modificationTime =
      file.fileTime(QFile::FileTime::FileModificationTime);
  if (!m_filesUpdates.contains(filePath)) {
    m_filesUpdates.insert(filePath,
                   FileChangeInfo{FileChangeType::added, modificationTime});
  } else if (m_filesUpdates[filePath].modificationTime != modificationTime) {
    m_filesUpdates[filePath].modificationTime = modificationTime;
    m_filesUpdates[filePath].type = FileChangeType::changed;
  } else {
    m_filesUpdates[filePath].type = FileChangeType::notChanged;
  }
}

void PictureProvider::setDirectory(const QString& directory) {
  if (m_directory != directory && QDir(directory).exists()) {
    markAllRemoved();
    m_directory = directory;
    updateFiles();
    emit directoryChanged();
    emit picturesChanged(getChanges());
    removeMarked();
  }
}

void PictureProvider::setLookupDepth(int depth) {
  if (m_lookupDepth != depth) {
    m_lookupDepth = depth;
    emit lookupDepthChanged();

    markAllRemoved();
    updateFiles();
    emit picturesChanged(getChanges());
    removeMarked();
  }
}

int PictureProvider::getLookupDepth() const noexcept {
  return m_lookupDepth;
}
