#ifndef QTHDF5_H
#define QTHDF5_H

#include <QString>
#include <QVector>
#include <QMetaType>
#include <QFile>

#include <exception>

#define HDF_EXPORT

class QH5Datatype;
class QH5Group;
class QH5Dataset;
class QH5File;

/**
 * @brief A wrapper for HDF5 object identifiers 
 * 
 * Implements part of the H5I API.
 * 
 */
class HDF_EXPORT QH5id
{
public:
    /**
     * @brief Define our type for storing id values
     */
    typedef qint64 h5id;

    /**
     * @brief Construct a new QH5id object
     * 
     * 
     * @param id HDF5 identifier
     * @param incref If true then the reference counter is incremented
     */
    explicit QH5id(h5id id = 0, bool incref = true);

    /**
     * @brief Copy constructor
     * 
     * o.id() is copied to the new object and the reference counter is
     * increased.
     * 
     * @param o Another QH5id object
     */
    QH5id(const QH5id& o);

    /**
     * @brief Destroy the QH5id object
     * 
     * Calls close().
     * 
     */
    ~QH5id() { close(); }

    /**
     * @brief Assignement operator.
     * 
     * The current HDF5 id of this object is released and
     * rhs.id() is copied over. 
     * The reference counter is increased.
     * 
     * @param rhs 
     * @return QH5id& 
     */
    QH5id& operator=(const QH5id& rhs);

    /**
     * @brief Check if this id is valid
     * 
     * Calls H5Iis_valid
     * 
     * @return true 
     * @return false 
     */
    bool isValid() const;

    /**
     * @brief Returns the stored id
     */
    const h5id& id() const { return id_; }

    /**
     * @brief Return the name of the corresponding id
     * 
     * Calls H5Iget_name
     * 
     * @return QByteArray 
     */
    QByteArray name() const;

    /**
     * @brief Close this id
     * 
     * If isValid() returns true, the type of identifier is queried
     * with H5Iget_type and then the appropriate H5Xclose function
     * is called.
     * 
     * @return true Identifier closed succesfully
     * @return false Invalid id or error while closing id
     */
    bool close();

    /**
     * @brief Promote to QH5Group
     * 
     * If the id refers to a valid HDF5 group a new QH5Group 
     * object is created and returned.
     * 
     * Otherwise an invalid object is returned.
     * 
     * @return QH5Group 
     */
    QH5Group toGroup() const;

    /**
     * @brief Promote to QH5Dataset
     * 
     * If the id refers to a valid HDF5 dataset a new QH5Dataset 
     * object is created and returned.
     * 
     * Otherwise an invalid object is returned.
     * 
     * @return QH5Dataset 
     */
    QH5Dataset toDataset() const;

    /**
     * @brief Return true if the id refers to a HDF5 group
     */
    bool isGroup() const;
    /**
     * @brief Return true if the id refers to a HDF5 dataset
     */
    bool isDataset() const;

protected:
    h5id id_;

    bool ref() const;
    bool deref() const;
    int refcount() const;
};

/**
 * @brief Return true if the 2 ids are equal.
 * 
 * The 2 ids must be valid and wrap the same HDF5 id.
 * 
 * @param lhs 
 * @param rhs 
 * @return HDF_EXPORT 
 */
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

/**
 * @brief A wrapper for a HDF5 dataspace
 * 
 */
class HDF_EXPORT QH5Dataspace : public QH5id
{
    friend class QH5Dataset;
    QH5Dataspace(h5id id, bool incref) : QH5id(id,incref) {}
public:

    /**
     * @brief Construct a new QH5Dataspace object from a dimensions vector 
     * 
     * The created dataspace has the following properties according to the 
     * value of dims :
     *      - dims.isEmpty()==true    : invalid dataspace
     *      - dims = {0}              : empty dataspace (H5S_NULL)
     *      - dims = {1}              : scalar dataspace (H5S_SCALAR)
     *      - dims = {n1,n2,...}      : simple dataspace (H5S_SIMPLE), n1 x n2 x ...
     * 
     * @param dims A vector of dimensions
     */
    QH5Dataspace(const QVector<quint64>& dims = QVector<quint64>());

    /**
     * @brief Return the dimensions of the HDF5 dataspace
     *
     * Uses the H5S API to query the dataspace dimensions.
     *
     * @return The result is returned as a QVector
     */
    QVector<quint64> dimensions() const;

    /**
     * @brief Returns the number of elements
     *
     * Calls H5Sget_simple_extent_npoints
     * 
     * @return int 
     */
    int size() const;

    /**
     * @brief Create a scalar dataspace
     */
    static QH5Dataspace scalar();
};

/**
 * @brief A wrapper for HDF5 datatypes
 */
class HDF_EXPORT QH5Datatype : public QH5id
{
    friend class QH5id;
    friend class QH5Node;
    friend class QH5Group;
    friend class QH5Dataset;

    QH5Datatype(h5id id, bool incref) : QH5id(id,incref) {}

    /*
     * Create from a Qt metatype id
     */
    static QH5Datatype fromMetaTypeId(int i);

    // helper template class providing specific datatype properties
    template<typename T>
    struct traits {
        static int metaTypeId(const T &) { return qMetaTypeId<T>(); }
        static QH5Dataspace dataspace(const T &){ return QVector<quint64>({1}); }
        static void resize(T &, int) {}
        static void *ptr(T &value) { return reinterpret_cast<void *>(&value); }
        static const void *cptr(const T &value) { return reinterpret_cast<const void *>(&value); }
    };

public:
    /**
     * @brief Enum type corresponding to H5T_class_t
     */
    enum Class {
        UNSUPPORTED,    //!< invalid or unsupported type
        INTEGER,        //!< integer type (H5T_INTEGER)
        FLOAT,          //!< float type (H5T_FLOAT)
        STRING          //!< string type (H5T_STRING)
    };

    /**
     * @brief Enum corresponding to H5T_cset_t
     */
    enum StringEncoding {
        ASCII,      //!< ASCII / Latin-1 encoding
        UTF8        //!< Unicode UTF8 encoding
    };

    /**
     * @brief Default constructor
     * 
     * Constructs an invalid QH5Datatype object
     * 
     */
    QH5Datatype() : QH5id() {}

    /**
     * @brief Construct a QH5Datatype object corresponding to v 's type
     */
    template<typename T>
    static QH5Datatype fromValue(const T& v)
    {
        return fromMetaTypeId(traits<T>::metaTypeId(v));
    }

    /**
     * @brief Get the Class of this datatype
     * 
     * Calls H5Tget_class
     */
    Class getClass() const;

    /**
     * @brief Get the Qt-metatype id corresponding to this datatype
     * 
     * If getClass() returns QH5Datatype::STRING then this function returns
     * QMetaType::QString.
     * 
     * Otherwise it calls H5Tget_native_type to obtain the native type of the HDF5
     * datatype and converts to an equivalent metatype id.  
     * 
     */
    int metaTypeId() const;

    /**
     * @brief Get the size of this datatype
     * 
     * Calls H5Tget_size
     * 
     * @return size_t 
     */
    size_t size() const;

    /**
     * @brief Get the String properties of this datatype
     * 
     * Calls H5Tget_cset & H5Tis_variable_str to obtain the string properties.
     * 
     * @param enc String encoding
     * @param size String size or size_t(-1) for variable (H5T_VARIABLE)
     * @return true If succesfull
     * @return false If the datatype is not string or an error occurred
     */
    bool getStringTraits(StringEncoding& enc, size_t &size) const;

    /**
     * @brief Set the String properties of this datatype
     * 
     * Calls H5Tset_cset & H5Tset_size to set the string properties.
     * 
     * @param enc String encoding
     * @param size String size or size_t(-1)  for variable (H5T_VARIABLE)
     * @return true If succesfull
     * @return false If the datatype is not string or an error occurred
     */
    bool setStringTraits(StringEncoding enc, size_t size) const;

    /**
     * @brief Construct a fixed-size string HDF5 datatype
     * 
     * This is a helper function to create a fixed-size string datatype.
     * 
     * String datatypes are otherwise by default created as variable-sized (i.e. zero terminated) 
     * 
     */
    static QH5Datatype fixedString(int size);

};

// specialization for vectors
template<typename T>
struct QH5Datatype::traits<QVector<T>> {
    static int metaTypeId(const QVector<T> &) { return qMetaTypeId<T>(); }
    static QH5Dataspace dataspace(const QVector<T> &value)
    { return QVector<quint64>(1,value.size()); }
    static void resize(QVector<T> & v, int n) { v.resize(n); }
    static void *ptr(QVector<T> &data) { return reinterpret_cast<void *>(data.data()); }
    static const void *cptr(const QVector<T> &data)
    { return reinterpret_cast<const void *>(data.constData()); }
};
// specialization for QString
template<>
struct QH5Datatype::traits<QString> {
    static int metaTypeId(const QString &) { return qMetaTypeId<QString>(); }
    static QH5Dataspace dataspace(const QString &)
    { return QVector<quint64>({1}); }
};
// specialization for QStringList
template<>
struct QH5Datatype::traits<QStringList> {
    static int metaTypeId(const QStringList &)
    { return qMetaTypeId<QString>(); }
    static QH5Dataspace dataspace(const QStringList & value)
    { return QVector<quint64>(1,value.size()); }
};

/**
 * @brief Represents a node in a HDF5 file
 * 
 * A QH5Node can be either a HDF5 group or a HDF5 dataset.
 * 
 * QH5Node offers access to HDF5 attributes. Currently only single-valued 
 * simple-type attributes are supported - no arrays or user-defined types
 * 
 */
class HDF_EXPORT QH5Node : public QH5id
{
    friend class QH5id;
    friend class QH5Dataset;
    friend class QH5Group;
    QH5Node(h5id id, bool incref) : QH5id(id,incref) {}
public:
    /**
     * @brief Default constructor
     * 
     * Constructs an invalid QH5Node object
     * 
     */
    QH5Node() : QH5id() {}

    /**
     * @brief Check if an attribute excists
     * 
     * @param name Attribute name
     * @return true If an attribute with this name excists
     * @return false Otherwise
     */
    bool hasAttribute(const char* name) const;
    /**
     * @brief Return the type of an attribute
     * 
     * @param name Name of the attribute
     * @return QH5Datatype The attribute's type or an empty QH5Datatype
     */
    QH5Datatype attributeType(const char* name) const;
    /**
     * @brief Get the names of all attributes
     * 
     * @return QByteArrayList 
     */
    QByteArrayList attributeNames() const;
    /**
     * @brief Read the value of an attribute
     * 
     * @tparam T Type of the attribute
     * @param name Attribute name
     * @param value Attribute value
     * @return true If succesfull
     * @return false Otherwise
     */
    template<typename T>
    bool readAttribute(const char* name, T& value) const {
        QH5Datatype datatype = QH5Datatype::fromValue(value);

        return readAttribute_(name, QH5Datatype::traits<T>::ptr(value),
                              QH5Datatype::fromValue(value));
    }
    /**
     * @brief Write the value of an attribute
     * 
     * If the attribute does not exist it is created
     * 
     * @tparam T Type of the attribute
     * @param name Attribute name
     * @param value Attribute value
     * @return true If succesfull
     * @return false Otherwise
     */
    template<typename T>
    bool writeAttribute(const char* name, const T& value) const {
        QH5Datatype datatype = QH5Datatype::fromValue(value);

        return writeAttribute_(name, QH5Datatype::traits<T>::cptr(value),
                              QH5Datatype::fromValue(value));
    }
private:
    bool readAttribute_(const char* name, void* data,
                        const QH5Datatype& memtype) const;
    bool writeAttribute_(const char* name, const void* data,
                        const QH5Datatype& memtype) const;
    bool readAttribute_(const char* name, QString& S) const;
    bool writeAttribute_(const char* name, const QString& S) const;
    QH5id openAttribute_(const char* name, const QH5Datatype& memtype, bool create) const;
};

template<>
inline bool QH5Node::readAttribute<QString>(const char* name, QString& value) const
{
    return readAttribute_(name, value);
};

template<>
inline bool QH5Node::writeAttribute<QString>(const char* name, const QString& value) const
{
    return writeAttribute_(name, value);
};

/**
 * @brief A wrapper for HDF5 datasets
 * 
 */
class HDF_EXPORT QH5Dataset : public QH5Node
{
    friend class QH5id;
    friend class QH5Group;
    QH5Dataset(h5id id, bool incref) : QH5Node(id,incref) {}

public:
    /**
     * @brief Default constructor
     * 
     * Creates an invalid QH5Dataset object
     * 
     */
    QH5Dataset() : QH5Node() {}

    QH5Dataset& operator=(const QH5Dataset& o)
    { *((QH5id*)this) = o; return *this; }

    /**
     * @brief Return the associated datatype
     */
    QH5Datatype datatype() const;

    /**
     * @brief Return the associated dataspace
     */
    QH5Dataspace dataspace() const;

    /**
     * @brief Write data to this dataset
     * 
     * Datatype and dataspace are inferred from data. 
     * 
     * @tparam T Type of the data
     * @param data data to write
     * @return true if data was written, false otherwise
     */
    template<typename T>
    bool write(const T& data) const
    {
        QH5Datatype datatype = QH5Datatype::fromValue(data);
        return write_(QH5Datatype::traits<T>::cptr(data),
                      QH5Datatype::traits<T>::dataspace(data),
                      datatype);
    }

    /**
     * @brief Write data to this dataset
     * 
     * @tparam T Type of the data
     * @param data data to write
     * @param memspace memory dataspace
     * @param memtype memory datatype
     * @return true if data was written, false otherwise
     */
    template<typename T>
    bool write(const T& data, const QH5Dataspace& memspace,
               const QH5Datatype& memtype) const
    {
        return write_(QH5Datatype::traits<T>::cptr(data),
                      memspace, memtype);
    }
    /**
     * @brief Write data to this dataset
     * 
     * The HDF5 datatype is inferred from type of the parameter data.
     * The HDF5 dataspace is scalar if data is a single value or 
     * a simple 1D dataspace is data is a QVector.
     * 
     * @tparam T Type of the data
     * @param data data to write
     * @return true if data was written, false otherwise
     */
    template<typename T>
    bool read(T& data) const
    {
        QH5Datatype datatype = QH5Datatype::fromValue(data);
        QH5Dataspace ds = dataspace();
        QH5Datatype::traits<T>::resize(data,ds.size());
        return read_(QH5Datatype::traits<T>::ptr(data),ds, datatype);
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

// template specializations of read/write functions
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

/**
 * @brief A wrapper for HDF5 groups
 * 
 */
class HDF_EXPORT QH5Group : public QH5Node
{
    friend class QH5id;
    friend class QH5File;
    QH5Group(h5id id, bool incref) : QH5Node(id,incref) {}
public:
    /**
     * @brief Default constructor
     * 
     * Create an invalid QH5Group object
     * 
     */
    QH5Group() : QH5Node() {}

    /**
     * @brief Check if a named link exists
     * 
     * Calls H5Lexists 
     * 
     * @param name name to check
     * @return true If name exists
     * @return false If this object is invalid or the name does not exists
     */
    bool exists(const char *name) const;

    /**
     * @brief Check if a named object is a dataset
     * 
     * First the function exists() is called to check if the named object exists.
     * Then the type of object is inferred by calling H5Oget_info_by_name
     * 
     * @param name name to check
     * @return true If name exists and is a dataset
     * @return false Otherwise
     */
    bool isDataset(const char *name) const;

    /**
     * @brief Check if a named object is a dataset
     * 
     * First the function exists() is called to check if the named object exists.
     * Then the type of object is inferred by calling H5Oget_info_by_name
     *       
     * @param name name to check
     * @return true If name exists and is a dataset
     * @return false Otherwise
     */
    bool isGroup(const char *name) const;

    /**
     * @brief Create a sub-group  
     * 
     * A new group with a given name is created under the current group, unless an object with the same name
     * exists or another error occurs.
     * 
     * If idxCreationOrder is set to true, then the creation order of items in the
     * sub-group is registered and will be enumerable.
     * 
     * @param name Name of the new group
     * @param idxCreationOrder Optional flag to register creation order
     * @return QH5Group The new group or an invalid object
     */
    QH5Group createGroup(const char *name, bool idxCreationOrder = false) const;

    /**
     * @brief Opens a sub-group 
     * 
     * The function checks if the name exists and if it is a group. Then it opens the group.
     * 
     * @param name Name of group to open
     * @return QH5Group The group wrapper object. Invalid if unsuccesfull.
     */
    QH5Group openGroup(const char *name) const;

    /**
     * @brief Returns true if creation order can be indexed in this group
     */
    bool isCreationOrderIdx() const;

    /**
     * @brief Create a dataset object
     * 
     * @param name The name of the dataset
     * @param dataspace The dataspace of the new dataset
     * @param datatype The datatype of the new dataset
     * @return QH5Dataset The dataset object. Invalid if the operation failed.
     */
    QH5Dataset createDataset(const char *name,
                             const QH5Dataspace& dataspace,
                             const QH5Datatype& datatype) const;

    /**
     * @brief Open a dataset
     * 
     * The function checks if the name exists and if it is a dataset. Then it opens the dataset. 
     * 
     * @param name The name of the dataset
     * @return QH5Dataset The dataset object. Invalid if the operation failed.
     */
    QH5Dataset openDataset(const char *name) const;

    /**
     * @brief Write data to a dataset
     * 
     * If the name corresponds to a dataset then it is opened otherwise a new dataset 
     * with this name is created.
     * 
     * The data is written to the dataset.
     * 
     * @tparam T Type of the data to write
     * @param name Name of the dataset
     * @param data The data to write
     * @return true If succesfull, false otherwise
     */
    template<typename T>
    bool write(const char *name, const T& data) const
    {
        QH5Dataset ds;
        if (exists(name) && isDataset(name)) ds = openDataset(name);
        else ds = createDataset(name, QH5Datatype::traits<T>::dataspace(data),
                                      QH5Datatype::fromValue(data));
        return ds.isValid() ? ds.write(data) : false;
    }

    /**
     * @brief Read data from a dataset
     * 
     * This function opens a dataset with the given name and reads
     * the stored data.
     * 
     * @tparam T Type of the data to read
     * @param name Name of the dataset
     * @param data Variable to store the data
     * @return true If succesfull, false otherwise
     */
    template<typename T>
    bool read(const char *name, T& data) const
    {
        QH5Dataset ds = openDataset(name);
        return ds.isValid() ? ds.read(data) : false;
    }

    /**
     * @brief Get all sub-groups of this group
     * 
     * @param idxCreationOrder If true and the group was created with creation order enabled then 
     *      the sub-groups are ordered according to creation time
     * @return QVector<QH5Group> A vector of all sub-groups of this group
     */
    QVector<QH5Group> subGroups(bool idxCreationOrder = false) const;

    /**
     * @brief Get all datasets in this group
     * 
     * @return QVector<QH5Dataset> A vector of all datasets
     */
    QVector<QH5Dataset> datasets() const;

    /**
     * @brief Get the names of all sub-groups of this group
     * 
     * @param idxCreationOrder If true and the group was created with creation order enabled then 
     *      the sub-group names are ordered according to creation time 
     * @return QByteArrayList A list of sub-group names
     */
    QByteArrayList groupNames(bool idxCreationOrder = false) const;

    /**
     * @brief Get the names of all datasets in this group
     * 
     * @return QByteArrayList A list of dataset names
     */
    QByteArrayList datasetNames() const;

private:
    QByteArray objname_by_idx(int i, bool idxCreationOrder) const;

};

/**
 * @brief A wrapper class for HDF5 files 
 * 
 */
class HDF_EXPORT QH5File
{
    friend class QH5id;

    QString fname_;
    QString error_msg_;
    QH5id id_;
public:
    /**
     * @brief Construct a new QH5File object
     * 
     * @param fname The name of the HDF5 file
     */
    QH5File (const QString& fname = QString()) : fname_(fname) {}

    /**
     * @brief Open the HDF5 file
     * 
     * If the file does not exist it is created.
     * 
     * If the file exists but mode is QIODevice::Truncate then it is opened and
     * truncated.
     * 
     * An existing HDF5 file can be opened either with QIODevice::ReadWrite or with QIODevice::ReadOnly.
     * 
     * @param mode A combination of QIODevice flags
     * @return true If the file has been succesfully opened or created
     * @return false File does not exist or could not be created
     */
    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);

    /**
     * @brief Close the HDF5 file. Returns true if succesfull. 
     */
    bool close() { return id_.close(); }

    /**
     * @brief Returns true if the file is open
     */
    bool isOpen() const { return id_.isValid(); }

    /**
     * @brief Set the filename of this object
     * 
     * If the file is already open then fname is ignored
     */
    void setFileName(const QString& fname)
    {
        if (!isOpen()) fname_ = fname;
    }

    /**
     * @brief Return the root group of the file
     * 
     * If the file is not open an invalid object is returned.
     * 
     * @return QH5Group The root group (named "/")
     */
    QH5Group root() const;

    /**
     * @brief Check if the disk file fname is a valid HDF5 file
     */
    static bool isHDF5(const QString& fname);

private:
    void pushError(const QString& err) { error_msg_ = err; }

};

class HDF_EXPORT h5exception : public std::runtime_error
{
public:
    h5exception(const char* msg) : std::runtime_error(msg)
    {}
};

#endif // QH5ID_H
