#ifndef QH5FILEMODEL_H
#define QH5FILEMODEL_H

#include <QAbstractItemModel>

#include "qthdf5.h"

class QFileIconProvider;

class QH5FileModel : public QAbstractItemModel
{
    Q_OBJECT

    void* rootNode;

    QH5File hdf5file;
    QFileIconProvider* iconProvider;

public:

    enum { NumColumns = 1 };

    explicit QH5FileModel(QObject *parent = 0);
    virtual ~QH5FileModel();

    void setFile(const QString& fname);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QString toString(const QModelIndex &index) const;


};

#endif // QH5FILEMODEL_H
