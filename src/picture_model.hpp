#pragma once
#include <QDir>
#include <QAbstractListModel>

class PictureListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  PictureListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  QString getDirectory() const;

  void setDirectory(QString directory);

  Q_PROPERTY(QString directory READ getDirectory WRITE setDirectory);

private:
  QString m_directory;
  QFileInfoList m_pictures;
};
