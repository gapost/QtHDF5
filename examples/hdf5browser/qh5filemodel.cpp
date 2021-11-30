#include "qh5filemodel.h"

#include <QImage>
#include <QIcon>
#include <QFileIconProvider>

class Node
{
public:
    QList<Node*> children;
    QByteArray name;
    QH5Node h5obj;
    Node *parent;

    explicit Node(const QByteArray& n, const QH5Node& obj, Node *parentItem = 0) :
        name(n),
        h5obj(obj),
        parent(parentItem)
    {}

    void populate()
    {
        if (!h5obj.isGroup()) return;
        QH5Group h5g = h5obj.toGroup();
        foreach (const QByteArray& name, h5g.groupNames()) {
            Node* n = new Node(name,h5g.openGroup(name),this);
            children.push_back(n);
            n->populate();
        }
        foreach (const QByteArray& name, h5g.datasetNames()) {
            Node* n = new Node(name,h5g.openDataset(name),this);
            children.push_back(n);
            n->populate();
        }
    }
    void deleteChildren()
    {
        foreach (Node* n, children) {
            n->deleteChildren();
            delete n;
        }
    }

    void appendChild(Node *child)
    {
        children.push_back(child);
    }
    void insertChild(int row, Node* child)
    {
        children.insert(row,child);
    }

    Node *child(int row)
    {
        return children.at(row);
    }
    int childCount() const
    {
        return children.size();
    }
    int row()
    {
        if (!parent) return -1;
        return parent->children.indexOf(this);
    }
};

QH5FileModel::QH5FileModel(QObject *parent) :
    QAbstractItemModel(parent), rootNode(0)
{
    iconProvider = new QFileIconProvider;
}

QH5FileModel::~QH5FileModel()
{
    if (rootNode) {
        Node* ptr = (Node*)rootNode;
        ptr->deleteChildren();
        delete ptr;
    }
    delete iconProvider;
}


void QH5FileModel::setFile(const QString& fname)
{
    if (rootNode) {
        Node* ptr = (Node*)rootNode;
        ptr->deleteChildren();
        delete ptr;
        rootNode = 0;
        hdf5file.close();
    }

    if (fname.isNull() || fname.isEmpty()) return;

    hdf5file.setFileName(fname);

    if (hdf5file.open(QIODevice::ReadOnly)) {
        Node* ptr = new Node("/",hdf5file.root());
        ptr->populate();
        rootNode = (void*)ptr;
    }
}

QVariant QH5FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        if (section == 0) {
            // ### TODO oh man this is ugly and doesn't even work all the way!
            // it is still 2 pixels off
            QImage pixmap(16, 1, QImage::Format_Mono);
            pixmap.fill(0);
            pixmap.setAlphaChannel(pixmap.createAlphaMask());
            return pixmap;
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QAbstractItemModel::headerData(section, orientation, role);

    QString returnValue;
    switch (section) {
    case 0: returnValue = tr("Name");
            break;
    case 1: returnValue = tr("Class");
            break;
    default: return QVariant();
    }

    return returnValue;
}

QModelIndex QH5FileModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent) || column >= columnCount(parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(0, column, rootNode);

    // get the parent node
    Node* parentNode = (Node*)parent.internalPointer();
    Q_ASSERT(parentNode);

    // now get the internal pointer for the index
    const Node* indexNode = parentNode->children.at(row);
    Q_ASSERT(indexNode);

    return createIndex(row, column, const_cast<Node*>(indexNode));
}

QModelIndex QH5FileModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
            return QModelIndex();

    Node* indexNode = (Node*)index.internalPointer();
    Q_ASSERT(indexNode != 0);

    if (indexNode == rootNode)
        return QModelIndex();

    Node* parentNode = indexNode->parent;
    Q_ASSERT(parentNode);

    if (parentNode == rootNode)
        return createIndex(0, 0, parentNode);

    // get the parent's row
    int row = parentNode->row();
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0, parentNode);
}

int QH5FileModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
         return 0;

    if (!parent.isValid()) return 1;

    Node* parentNode = (Node*)parent.internalPointer();
    Q_ASSERT(parentNode);

    return parentNode->childCount();
}

int QH5FileModel::columnCount(const QModelIndex &parent) const
{
    return (parent.column() > 0) ? 0 : NumColumns;
}

bool QH5FileModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) return true;

    Node* parentNode = (Node*)parent.internalPointer();
    Q_ASSERT(parentNode);

    return (!parentNode->children.isEmpty());
}

bool QH5FileModel::canFetchMore(const QModelIndex &parent) const
{
    // FIXME: Implement me!
    Q_UNUSED(parent)
    return false;
}

void QH5FileModel::fetchMore(const QModelIndex &parent)
{
    // FIXME: Implement me!
    Q_UNUSED(parent)
}

QVariant QH5FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this)
        return QVariant();

    Node* nd = (Node*)index.internalPointer();
    Q_ASSERT(nd);

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return nd->name;
        case 1:
            if (nd->h5obj.isGroup()) return "Group";
            else if (nd->h5obj.isDataset()) return "Dataset";
            else return "";
        default:
            qWarning("data: invalid display value column %d", index.column());
            break;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (nd->h5obj.isGroup())
                return iconProvider->icon(QFileIconProvider::Folder);
            else if (nd->h5obj.isDataset())
                return iconProvider->icon(QFileIconProvider::File);
            else return QIcon();
        }
        break;
//    case Qt::TextAlignmentRole:
//        if (index.column() == 1)
//            return Qt::AlignRight;
//        break;
    }

    return QVariant();
}

QH5Node QH5FileModel::h5node(const QModelIndex &index) const
{
    Node* nd = (Node*)index.internalPointer();
    Q_ASSERT(nd);

    return nd->h5obj;
}

