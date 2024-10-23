#include "picture_model.hpp"

PictureListModel::PictureListModel(QObject* parent)
: QAbstractListModel(parent), m_pictures({})
{}

int PictureListModel::rowCount(const QModelIndex& parent) const
{
  //
  if (! parent.parent().isValid()) {
    return m_pictures.size();
  }
  return 0;
}

QVariant PictureListModel::data(const QModelIndex &index, int role) const
{
  bool fValidQuery = index.isValid() && role == Qt::ItemDataRole::DisplayRole
        && index.row() < m_pictures.size();
  if (! fValidQuery) {
    return {};
  }
  return m_pictures[index.row()].fileName();
}

QString PictureListModel::getDirectory() const
{
  return m_directory;
}

void PictureListModel::setDirectory(QString directory)
{
  m_directory = directory;
  m_pictures = QDir(m_directory).entryInfoList();
}
