#ifndef QH5FILEMODEL_H
#define QH5FILEMODEL_H

#include <QAbstractItemModel>

#include "qthdf5.h"

class QFileIconProvider;

/**
 * @brief Defines a Qt item model for HDF5 files
 *
 * This class can be used for presenting HDF5 data in Qt forms and UI elements.
 *
 * First instantiate a QH5FileModel and pass it a HDF5 file name:
 *
 * \code
 * QH5FileModel model;
 * model.setFile("FILENAME.H5");
 * \endcode
 *
 * The item model is internally populated with all nodes (groups, datasets) in the file.
 *
 * If we assign the model to a view class, the structure of the file will be displayed.
 *
 * \code
 * QTreeView view;
 * view.setModel(model);
 * \endcode
 *
 * We can access the HDF5 data stored in the QH5FileModel by means of a QModelIndex
 * and calling the function h5node(const QModelIndex&)
 *
 * \code
 * QModelIndex rootIdx = model.index(0,0);
 * QH5Node model.h5node(rootIdx); // this refers to the root group "/"
 * \endcode
 *
 * A model index is typically obtained from signals emitted by view classes
 * when the user clicks or selects a specific HDF5 node on the UI.
 *
 * The model class currently opens the file in read-only mode and thus can serve only
 * display purposes and not for editing.
 *
 * Calling setFile("") or deleting the model object automatically closes all references to the HDF5 file.
 *
 */
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

    /*!
     * \brief Set the HDF5 file for the model
     * \param fname: Name of the HDF5 file.
     *
     * The file should be accesible and a valid HDF5 file. Otherwise the
     * model will be empty.
     *
     * Passing an empty or null string will reset the model
     * and remove all content.
     *
     */
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

    /**
     * @brief Returns the QH5Node associated with a model index.
     *
     * This function can be used in order to get the HDF5 node
     * that corresponds to a specific model index.
     *
     * This is usefull in code called in response to
     * item view signals, e.g., activated(const QModelIndex&), to
     * obtain a handle the HDF5 data associated with a given model index.
     */
    QH5Node h5node(const QModelIndex &index) const;


};

#endif // QH5FILEMODEL_H
