#ifndef QTHDF5_H
#define QTHDF5_H

#include <QString>
#include <QVector>
#include <QMetaType>
#include <QFile>

#define HDF_EXPORT

class QH5Datatype;
class QH5Group;
class QH5Dataset;
class QH5File;

class HDF_EXPORT QH5id
{
public:
    typedef qint64 h5id;
    explicit QH5id(h5id id = 0, bool incref = true);
    QH5id(const QH5id& o);
    ~QH5id() { close(); }

    QH5id& operator=(const QH5id& o);

    bool isValid() const;
    const h5id& id() const { return id_; }
    QByteArray name() const;

    bool close();

    QH5Group toGroup() const;
    QH5Dataset toDataset() const;
    bool isGroup() const;
    bool isDataset() const;

protected:
    h5id id_;
    bool ref() const;
    bool deref() const;
    int refcount() const;
};

HDF_EXPORT
inline bool operator==(const QH5id &lhs, const QH5id &rhs)
{
  if ((!rhs.isValid()) || (!lhs.isValid())) return false;
  return lhs.id() == rhs.id();
}
HDF_EXPORT
inline bool operator!=(const QH5id &lhs, const QH5id &rhs)
{
  return !(lhs==rhs);
}

class QH5Dataspace : public QH5id
{
    friend class QH5Dataset;
    QH5Dataspace(h5id id, bool incref) : QH5id(id,incref) {}
public:
    /*
     * QVector<quint64>& dims:
     *   isEmpty() : invalid dataspace
     *   [0]       : empty dataspace H5S_NULL
     *   [1]       : scalar H5S_SCALAR
     *   [n,m,...] : simple H5S_SIMPLE
     */
    QH5Dataspace(const QVector<quint64>& dims = QVector<quint64>());
    QH5Dataspace(const QH5Dataspace& g) : QH5id(g) {}
    ~QH5Dataspace() {}

    QVector<quint64> dimensions() const;
    int size() const;
};

template<typename T>
class TypeTraits {
public:

    static int metaTypeId(const T &) { return qMetaTypeId<T>(); }

    static QH5Dataspace dataspace(const T &)
    { return QVector<quint64>({1}); }

    static void resize(T &, int) {}

    static void *ptr(T &value) {
        return reinterpret_cast<void *>(&value);
    }

    static const void *cptr(const T &value) {
        return reinterpret_cast<const void *>(&value);
    }
};

template<typename T>
class TypeTraits<QVector<T>> {
public:

    static int metaTypeId(const QVector<T> &)
    { return qMetaTypeId<T>(); }

    static QH5Dataspace dataspace(const QVector<T> &value)
    { return QVector<quint64>(1,value.size()); }

    static void resize(QVector<T> & v, int n)
    { v.resize(n); }

    static void *ptr(QVector<T> &data) {
        return reinterpret_cast<void *>(data.data());
    }

    static const void *cptr(const QVector<T> &data) {
        return reinterpret_cast<const void *>(data.constData());
    }
};

template<>
class TypeTraits<QString> {
public:

    static int metaTypeId(const QString &)
    { return qMetaTypeId<QString>(); }

    static QH5Dataspace dataspace(const QString &)
    { return QVector<quint64>({1}); }
};

template<>
class TypeTraits<QStringList> {
public:

    static int metaTypeId(const QStringList &)
    { return qMetaTypeId<QString>(); }

    static QH5Dataspace dataspace(const QStringList & value)
    { return QVector<quint64>(1,value.size()); }
};

class QH5Datatype : public QH5id
{
    friend class QH5id;
    friend class QH5Group;
    friend class QH5Dataset;
    QH5Datatype(h5id id, bool incref) : QH5id(id,incref) {}
    static QH5Datatype fromMetaTypeId(int i);
public:
    enum Class {
        UNSUPPORTED,
        INTEGER,
        FLOAT,
        STRING
    };

    enum StringEncoding {
        ASCII,
        UTF8
    };

    QH5Datatype() : QH5id() {}
    QH5Datatype(const QH5Datatype& g) : QH5id(g) {}
    ~QH5Datatype() {}

    template<typename T>
    static QH5Datatype fromValue(const T& v)
    {
        return fromMetaTypeId(TypeTraits<T>::metaTypeId(v));
    }

    Class getClass() const;

    int metaTypeId() const;

    size_t size() const;
    bool getStringTraits(StringEncoding& enc, size_t &size) const;
    bool setStringTraits(StringEncoding enc, size_t size) const;

    static QH5Datatype fixedString(int size);
};



class QH5Dataset : public QH5id
{
    friend class QH5id;
    friend class QH5Group;
    QH5Dataset(h5id id, bool incref) : QH5id(id,incref) {}

public:
    QH5Dataset() : QH5id() {}
    QH5Dataset(const QH5Dataset& g) : QH5id(g) {}
    ~QH5Dataset() {}

    QH5Datatype datatype() const;
    QH5Dataspace dataspace() const;

    template<typename T>
    bool write(const T& data) const
    {
        QH5Datatype datatype = QH5Datatype::fromValue(data);
        return write_(TypeTraits<T>::cptr(data),
                      TypeTraits<T>::dataspace(data),
                      datatype);
    }
    template<typename T>
    bool write(const T& data, const QH5Dataspace& memspace,
               const QH5Datatype& memtype) const
    {
        return write_(TypeTraits<T>::cptr(data),
                      memspace, memtype);
    }
    template<typename T>
    bool read(T& data) const
    {
        QH5Datatype datatype = QH5Datatype::fromValue(data);
        QH5Dataspace ds = dataspace();
        TypeTraits<T>::resize(data,ds.size());
        return read_(TypeTraits<T>::ptr(data),ds, datatype);
    }

private:
    bool write_(const void* data, const QH5Dataspace& memspace,
               const QH5Datatype& memtype) const;
    bool write_(const QString& str) const;
    bool write_(const QString& str, const QH5Dataspace& memspace,
                const QH5Datatype& memtype) const;
    bool write_(const QStringList& str) const;
    bool write_(const QStringList& str, const QH5Dataspace& memspace,
                const QH5Datatype& memtype) const;
    bool read_(void* data, const QH5Dataspace& memspace,
              const QH5Datatype& memtype) const;
    bool read_(QString& str) const;
    bool read_(QStringList& str) const;
};


template<>
inline bool QH5Dataset::read<QString>(QString& data) const
{
    return read_(data);
};
template<>
inline bool QH5Dataset::read<QStringList>(QStringList& data) const
{
    return read_(data);
};
template<>
inline bool QH5Dataset::write<QString>(const QString& data) const
{
    return write_(data);
};
template<>
inline bool QH5Dataset::write<QString>(const QString& data, const QH5Dataspace& memspace,
                                       const QH5Datatype& memtype) const
{
    return write_(data, memspace, memtype);
};
template<>
inline bool QH5Dataset::write<QStringList>(const QStringList& data) const
{
    return write_(data);
};
template<>
inline bool QH5Dataset::write<QStringList>(const QStringList& data, const QH5Dataspace& memspace,
                                           const QH5Datatype& memtype) const
{
    return write_(data, memspace, memtype);
};

class QH5Group : public QH5id
{
    friend class QH5id;
    friend class QH5File;
    QH5Group(h5id id, bool incref) : QH5id(id,incref) {}
public:
    QH5Group() : QH5id() {}
    QH5Group(const QH5Group& g) : QH5id(g) {}
    ~QH5Group() {}

    bool exists(const char *name) const;
    bool isDataset(const char *name) const;
    bool isGroup(const char *name) const;

    QH5Group createGroup(const char *name) const;
    QH5Group openGroup(const char *name) const;

    QH5Dataset createDataset(const char *name,
                             const QH5Dataspace& memspace,
                             const QH5Datatype& datatype) const;
    QH5Dataset openDataset(const char *name) const;

    template<typename T>
    bool write(const char *name, const T& data) const
    {
        QH5Dataset ds = createDataset(name, TypeTraits<T>::dataspace(data),
                                      QH5Datatype::fromValue(data));
        return ds.isValid() ? ds.write(data) : false;
    }
    template<typename T>
    bool read(const char *name, T& data) const
    {
        QH5Dataset ds = openDataset(name);
        return ds.isValid() ? ds.read(data) : false;
    }

    QVector<QH5Group> subGroups() const;
    QVector<QH5Dataset> datasets() const;

    QByteArrayList groupNames() const;
    QByteArrayList datasetNames() const;

private:
    QByteArray objname_by_idx(int i) const;

};

class QH5File
{
    QString fname_;
    QString error_msg_;
    QH5id id_;
public:

    QH5File (const QString& fname = QString()) : fname_(fname) {}
    ~QH5File () {}

    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);
    bool close() { return id_.close(); }

    bool isOpen() const { return id_.isValid(); }

    void setFileName(const QString& fname)
    {
        if (!isOpen()) fname_ = fname;
    }

    QH5Group root() const;

    static bool isHDF5(const QString& fname);

};

#endif // QH5ID_H
