#include "qthdf5.h"

#include <hdf5.h>

#include <QtDebug>

hid_t _h(const QH5id::h5id& v) { return static_cast<hid_t>(v); }
hid_t _h(const QH5id& v) { return static_cast<hid_t>(v.id()); }

QH5id::QH5id(h5id id, bool incref) : id_(id)
{
    if (id_ > 0 && incref) ref();
}

QH5id::QH5id(const QH5id& o) : id_(o.id_)
{
    if (isValid()) ref();
}

QH5id& QH5id::operator=(const QH5id& o)
{
    if (this == &o) return *this;

    if (isValid() && !close()) {
        // error closing
    }

    if (o.isValid() && !o.ref()) {
        // error ref
    }

    id_ = o.id_;

    return *this;
}

bool QH5id::close()
{
    if (isValid()) {
        H5I_type_t type = H5Iget_type(_h(id_));
        herr_t error_code = 0;

        switch (type)
        {
        case H5I_DATASPACE:
            error_code = H5Sclose(_h(id_));
            break;
        case H5I_GROUP:
            error_code = H5Gclose(_h(id_));
            break;
        case H5I_DATATYPE:
            error_code = H5Tclose(_h(id_));
            break;
        case H5I_ATTR:
            error_code = H5Aclose(_h(id_));
            break;
        case H5I_FILE:
            error_code = H5Fclose(_h(id_));
            break;
        case H5I_GENPROP_LST:
            error_code = H5Pclose(_h(id_));
            break;
        case H5I_GENPROP_CLS:
            error_code = H5Pclose_class(_h(id_));
            break;
        case H5I_ERROR_MSG:
            error_code = H5Eclose_msg(_h(id_));
            break;
        case H5I_ERROR_STACK:
            error_code = H5Eclose_stack(_h(id_));
            break;
        case H5I_ERROR_CLASS:
            error_code = H5Eunregister_class(_h(id_));
            break;
        default:
            error_code = H5Oclose(_h(id_));
        }

        if (error_code < 0) {
            // register error
            id_ = 0;
            return false;
        } else {
            id_ = 0;
            return true;
        }
    }
    return false;
}

bool QH5id::isValid() const
{
    if (id_ == 0) return false;

    htri_t ret = H5Iis_valid(_h(id_));

    //  ret=0 -> invalid, (ret < 0) -> error

    return (ret>0);
}
QByteArray QH5id::name() const
{
    ssize_t sz = H5Iget_name( _h(id_), NULL, 0 );
    if (sz) {
        QByteArray ba(sz,'\0');
        H5Iget_name( _h(id_), ba.data(), sz+1 );
        return ba;
    } else return QByteArray();
}
bool QH5id::isGroup() const
{
    return H5Iget_type(_h(id_))==H5I_GROUP;
}
bool QH5id::isDataset() const
{
    return H5Iget_type(_h(id_))==H5I_DATASET;
}
QH5Group QH5id::toGroup() const
{
    return isGroup() ? QH5Group(_h(id_),true) : QH5Group();
}
QH5Dataset QH5id::toDataset() const
{
    return isDataset() ? QH5Dataset(_h(id_),true) : QH5Dataset();
}
bool QH5id::ref() const
{
    return H5Iinc_ref(_h(id_))>=0;
}

bool QH5id::deref() const
{
    return H5Idec_ref(_h(id_))>=0;
}

int QH5id::refcount() const
{
    return H5Iget_ref(_h(id_));
}


/************* DATASPACE **************/
QH5Dataspace::QH5Dataspace(const QVector<quint64>& dims)
{
    if (!dims.isEmpty()) {
        hid_t space_id;
        if (dims.size()==1 && dims[0]==0) space_id = H5Screate(H5S_NULL);
        else if (dims.size()==1 && dims[0]==1) space_id = H5Screate(H5S_SCALAR);
        else space_id = H5Screate_simple (dims.size(), dims.constData(), NULL);
        id_ = static_cast<h5id>(space_id);
    }
}
QVector<quint64> QH5Dataspace::dimensions() const
{
    QVector<quint64> dims;
    if (!isValid()) return dims;

    switch (H5Sget_simple_extent_type(_h(id_)))
    {
    case H5S_SIMPLE:
        dims.resize(H5Sget_simple_extent_ndims(_h(id_)));
        H5Sget_simple_extent_dims(_h(id_), dims.data(), NULL);
        break;
    case H5S_SCALAR:
        dims = {1};
        break;
    case H5S_NULL:
        dims = {0};
        break;
    default:
        return QVector<quint64>();
    }

    return dims;
}
int QH5Dataspace::size() const
{
    hssize_t s = H5Sget_simple_extent_npoints(_h(id_));
    if (s < 0) {
        // error
        return 0;
    }
    return s;
}
/************* DATATYPE ***************/
QH5Datatype QH5Datatype::fromMetaTypeId(int i)
{
    QMetaType::Type metatype = static_cast<QMetaType::Type>(i);

    if (metatype==QMetaType::QString) {
        QH5Datatype datatype(H5Tcopy(H5T_C_S1),false);
        datatype.setStringTraits(UTF8,H5T_VARIABLE);
        return datatype;
    }

    switch (metatype)
    {
    case QMetaType::Bool:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_HBOOL),false);
    case QMetaType::Char:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_CHAR),false);
    case QMetaType::SChar:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_SCHAR),false);
    case QMetaType::UChar:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_UCHAR),false);
    case QMetaType::Short:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_SHORT),false);
    case QMetaType::UShort:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_USHORT),false);
    case QMetaType::Int:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_INT),false);
    case QMetaType::UInt:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_UINT),false);
    case QMetaType::Long:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_LONG),false);
    case QMetaType::ULong:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_ULONG),false);
    case QMetaType::LongLong:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_LLONG),false);
    case QMetaType::ULongLong:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_ULLONG),false);

    case QMetaType::Float:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_FLOAT),false);
    case QMetaType::Double:
        return QH5Datatype(H5Tcopy(H5T_NATIVE_DOUBLE),false);

    default:
        return QH5Datatype();
    }
}
int QH5Datatype::metaTypeId() const
{
    if (getClass()==STRING) return QMetaType::QString;

    hid_t id = H5Tget_native_type(_h(id_), H5T_DIR_ASCEND);
    if (id < 0) {
        // error
        return QMetaType::UnknownType;
    }
    if (id == H5T_NATIVE_CHAR)          return QMetaType::Char;
    else if (id == H5T_NATIVE_SCHAR)    return QMetaType::SChar;
    else if (id == H5T_NATIVE_SHORT)    return QMetaType::Short;
    else if (id == H5T_NATIVE_INT)      return QMetaType::Int;
    else if (id == H5T_NATIVE_LONG)     return QMetaType::Long;
    else if (id == H5T_NATIVE_LLONG)    return QMetaType::LongLong;
    else if (id == H5T_NATIVE_UCHAR)    return QMetaType::UChar;
    else if (id == H5T_NATIVE_USHORT)   return QMetaType::UShort;
    else if (id == H5T_NATIVE_UINT)     return QMetaType::UInt;
    else if (id == H5T_NATIVE_ULONG)    return QMetaType::ULong;
    else if (id == H5T_NATIVE_ULLONG)   return QMetaType::ULongLong;
    else if (id == H5T_NATIVE_FLOAT)    return QMetaType::Float;
    else if (id == H5T_NATIVE_DOUBLE)   return QMetaType::Double;
    else if (id == H5T_NATIVE_B8)       return qMetaTypeId<quint8>();
    else if (id == H5T_NATIVE_B16)      return qMetaTypeId<quint16>();
    else if (id == H5T_NATIVE_B32)      return qMetaTypeId<quint32>();
    else if (id == H5T_NATIVE_B64)      return qMetaTypeId<quint64>();
    else return QMetaType::UnknownType;
}
QH5Datatype::Class QH5Datatype::getClass() const
{
    H5T_class_t h5class = H5Tget_class(_h(id_));
    switch (h5class)
    {
    case H5T_INTEGER:
        return INTEGER;
    case H5T_FLOAT:
        return FLOAT;
    case H5T_STRING:
        return STRING;
    default:
        return UNSUPPORTED;
    }
}
bool QH5Datatype::getStringTraits(StringEncoding& enc, size_t& sz) const
{
    if (getClass()!=QH5Datatype::STRING) return false;
    {
        H5T_cset_t ret = H5Tget_cset(_h(id_));
        if (ret<0)
        {
            // error
            return false;
        }
        enc = (ret==H5T_CSET_ASCII) ? ASCII : UTF8;
    }
    {
        htri_t ret = H5Tis_variable_str(_h(id_));
        if (ret <0)
        {
            // error
            return false;
        }
        sz = (0 != ret) ? H5T_VARIABLE : size();
    }

    return true;
}
bool QH5Datatype::setStringTraits(StringEncoding enc, size_t sz) const
{
    if (getClass()!=QH5Datatype::STRING) return false;
    if (sz==0) return false;

    herr_t ret = H5Tset_cset(_h(id_),
                             enc==ASCII ? H5T_CSET_ASCII : H5T_CSET_UTF8);
    if (ret<0)
    {
        // error
        return false;
    }

    ret = H5Tset_size(_h(id_), sz );
    if (ret <0)
    {
        // error
        return false;
    }

    return true;
}
size_t QH5Datatype::size() const
{
    size_t s = H5Tget_size(_h(id_));
    if (s == 0) {
        // error
    }
    return s;
}
QH5Datatype QH5Datatype::fixedString(int size)
{
    QH5Datatype datatype(H5Tcopy(H5T_C_S1),false);
    datatype.setStringTraits(UTF8,size);
    return datatype;
}
/*********** DATASET ************/
bool QH5Dataset::write_(const void* data, const QH5Dataspace& memspace,
                       const QH5Datatype& memtype) const
{
    if (!data || !memspace.isValid() || !memtype.isValid()) return false;

    return H5Dwrite (_h(id_), _h(memtype.id()), _h(memspace.id()),
                     H5S_ALL, H5P_DEFAULT, data) >= 0;
}
bool QH5Dataset::write_(const QString& str) const
{
    return write_(str, QH5Dataspace({1}), datatype());
}
bool QH5Dataset::write_(const QString &str, const QH5Dataspace &memspace, const QH5Datatype &memtype) const
{
    if (memtype.getClass() != QH5Datatype::STRING) return false;
    size_t sz;
    QH5Datatype::StringEncoding enc;
    memtype.getStringTraits(enc,sz);
    QByteArray buff = (enc==QH5Datatype::ASCII) ? str.toLatin1() : str.toUtf8();
    if (sz==H5T_VARIABLE) {
        char* p[1] = { buff.data() };
        herr_t ret = H5Dwrite (_h(id_), _h(memtype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, p);
        if (ret < 0) {
            // error
            return false;
        }
        return true;
    } else {
        if (buff.size()+1>(int)sz) {
            // error: string too large for dataset
            return false;
        } else if (buff.size()+1<(int)sz) {
            // zero pad
            int n = sz - buff.size() - 1;
            buff.append(n,'\0');
        }
        return H5Dwrite (_h(id_), _h(memtype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, buff.constData()) >= 0;
    }

}
bool QH5Dataset::write_(const QStringList& str) const
{
    return write_(str, QH5Dataspace(QVector<quint64> (1,str.size())), datatype());
}
bool QH5Dataset::write_(const QStringList& str, const QH5Dataspace &memspace, const QH5Datatype &memtype) const
{
    if (memtype.getClass() != QH5Datatype::STRING) return false;
    size_t sz;
    QH5Datatype::StringEncoding enc;
    memtype.getStringTraits(enc,sz);

    if (sz==H5T_VARIABLE) {
        QVector<char*> vbuff(str.size());
        QByteArrayList ba;
        int i=0;
        foreach(const QString& s, str) {
            ba.push_back((enc==QH5Datatype::ASCII) ? s.toLatin1() : s.toUtf8());
            vbuff[i++] = ba.last().data();
        }
        herr_t ret = H5Dwrite (_h(id_), _h(memtype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, vbuff.data());
        if (ret < 0) {
            // error
            return false;
        }
        return true;

    } else {
        QByteArray buff((int)sz*str.size(),'\0');
        char* p = buff.data();
        foreach(const QString& s, str) {
            QByteArray ba = (enc==QH5Datatype::ASCII) ? s.toLatin1() : s.toUtf8();
            if (ba.size()+1>(int)sz) {
                // error: string too large for dataset
                return false;
            }
            memcpy(p, ba.constData(), ba.size());
            p += sz;
        }
        return H5Dwrite (_h(id_), _h(memtype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, buff.constData()) >= 0;
    }
}
bool QH5Dataset::read_(void* data, const QH5Dataspace &memspace,
                      const QH5Datatype& memtype) const
{
    if (!data || !memspace.isValid() || !memtype.isValid()) return false;

    return H5Dread (_h(id_), _h(memtype.id()), _h(memspace.id()),
                    H5S_ALL, H5P_DEFAULT, data) >= 0;
}
bool QH5Dataset::read_(QString& str) const
{
    QH5Dataspace memspace({1});
    QH5Datatype filetype = datatype();
    if (filetype.getClass() != QH5Datatype::STRING) return false;
    size_t sz;
    QH5Datatype::StringEncoding enc;
    filetype.getStringTraits(enc,sz);

    if (sz==H5T_VARIABLE) {
        char* p;
        herr_t ret =  H5Dread (_h(id_), _h(filetype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, &p) >= 0;
        if (ret < 0) {
            //error
            return false;
        }
        str = (enc==QH5Datatype::ASCII) ? QString::fromLatin1(p) :
                                          QString::fromUtf8(p);
        ret = H5Dvlen_reclaim (_h(filetype.id()), _h(memspace.id()), H5P_DEFAULT, &p);
        if (ret < 0) {
            // error
        }
        return true;
    } else {
        QByteArray buff(sz,'\0');
        return H5Dread (_h(id_), _h(filetype.id()), _h(memspace.id()),
                         H5S_ALL, H5P_DEFAULT, buff.data()) >= 0;
    }

}
bool QH5Dataset::read_(QStringList& str) const
{
    QH5Dataspace ds = dataspace();
    QVector<quint64> dims = ds.dimensions();
    if (dims.size()>1) {
        // error
        return false;
    }
    int n = dims[0];
    QH5Datatype filetype = datatype();
    if (filetype.getClass() != QH5Datatype::STRING) return false;
    size_t sz;
    QH5Datatype::StringEncoding enc;
    filetype.getStringTraits(enc,sz);

    if (sz==H5T_VARIABLE) {
        QVector<char*> p(n);
        herr_t ret =  H5Dread (_h(id_), _h(filetype.id()), _h(ds.id()),
                         H5S_ALL, H5P_DEFAULT, p.data()) >= 0;
        if (ret < 0) {
            //error
            return false;
        }
        for(int i = 0; i<n; i++) {
            QString s = (enc==QH5Datatype::ASCII) ? QString::fromLatin1(p[i]) :
                                                    QString::fromUtf8(p[i]);
            str.push_back(s);
        }
        ret = H5Dvlen_reclaim (_h(filetype.id()), _h(ds.id()), H5P_DEFAULT, p.data());
        if (ret < 0) {
            // error
        }
        return true;
    } else {
        QByteArray buff((int)sz*n,'\0');

        herr_t ret = H5Dread (_h(id_), _h(filetype.id()), _h(ds.id()),
                         H5S_ALL, H5P_DEFAULT, buff.data());
        if (ret < 0) {
            // error
            return false;
        }

        const char* p = buff.data();
        for(int i=0; i<n; i++) {
            QString s = (enc==QH5Datatype::ASCII) ? QString::fromLatin1(p) :
                                                    QString::fromUtf8(p);
            str.push_back(s);
            p += sz;
        }
        return true;
    }
}
QH5Datatype QH5Dataset::datatype() const
{
    hid_t id = H5Dget_type(_h(id_));
    if(id<0) {
        // error
    }
    return QH5Datatype(static_cast<h5id>(id),false);
}
QH5Dataspace QH5Dataset::dataspace() const
{
    hid_t id = H5Dget_space(_h(id_));
    if(id<0) {
        // error
    }
    return QH5Dataspace(static_cast<h5id>(id),false);
}
/*********** FILE ************/
bool QH5File::isHDF5(const QString& fname)
{
    return H5Fis_hdf5 (fname.toLatin1()) > 0;
}

bool QH5File::open(QIODevice::OpenMode mode)
{
    if (isOpen()) {
        error_msg_ = QString("The file '%1' is already open").arg(fname_);
        return false;
    }

    if (fname_.isEmpty()) return false;

    bool bExists = QFile::exists(fname_);

    hid_t fid;
    if ( !bExists || (bExists && mode.testFlag(QIODevice::Truncate)))
        fid = H5Fcreate (fname_.toLatin1(),
                         H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    else {
        if (!isHDF5(fname_)) {
            error_msg_ = QString("The file %1 is not in the HDF5 format").arg(fname_);
            return false;
        }

        unsigned int flags = H5F_ACC_RDWR;
        if (mode.testFlag(QIODevice::ReadOnly) && !mode.testFlag(QIODevice::WriteOnly))
            flags = H5F_ACC_RDONLY;
        fid = H5Fopen (fname_.toLatin1(), flags, H5P_DEFAULT);
    }

    if (fid < 0) {
        error_msg_ = QString("Opening the file %1 failed, %2").arg(fname_).arg(QString(strerror (errno)));
        return false;
    } else {
        id_ = QH5id(_h(fid), false); // create id obj with refcount = 1
    }

    return id_.isValid();
}
QH5Group QH5File::root() const
{
    if (!isOpen()) return QH5Group();
    hid_t gid = H5Gopen(_h(id_), "/", H5P_DEFAULT);
    return QH5Group(static_cast<QH5id::h5id>(gid), false);
}
/********** GROUP *****************/
bool QH5Group::exists(const char *name) const
{
    return isValid() && H5Lexists (_h(id_),name,H5P_DEFAULT);
}
bool QH5Group::isDataset(const char *name) const
{
    if (!exists(name)) return false;
    // use compatibility function for HDF5 <= 1.10
#if H5_VERSION_GE(1,12,0)
    H5O_info1_t info;
    H5Oget_info_by_name1(_h(id_),name,&info,0);
#else
    H5O_info_t info;
    H5Oget_info_by_name(_h(id_),name,&info,0);
#endif
    return info.type == H5O_TYPE_DATASET;
}
bool QH5Group::isGroup(const char *name) const
{
    if (!exists(name)) return false;

    // use compatibility function for HDF5 <= 1.10
#if H5_VERSION_GE(1,12,0)
    H5O_info1_t info;
    H5Oget_info_by_name1(_h(id_),name,&info,0);
#else
    H5O_info_t info;
    H5Oget_info_by_name(_h(id_),name,&info,0);
#endif

    return info.type == H5O_TYPE_GROUP;
};
QH5Group QH5Group::createGroup(const char *name) const
{
    if (exists(name)) {
        // error
        return QH5Group();
    }
    hid_t gid = H5Gcreate(_h(id_), name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (gid < 0) {
        // error
    }
    return QH5Group(static_cast<QH5id::h5id>(gid), false);
}
QH5Group QH5Group::openGroup(const char *name) const
{
    if (!isGroup(name)) {
        // error
        return QH5Group();
    }
    hid_t gid = H5Gopen(_h(id_), name, H5P_DEFAULT);
    if (gid < 0) {
        // error
    }
    return QH5Group(static_cast<QH5id::h5id>(gid), false);
}
QH5Dataset QH5Group::createDataset(const char *name,
                         const QH5Dataspace& memspace,
                         const QH5Datatype& datatype) const
{
    if (exists(name)) {
        // error
        return QH5Dataset();
    }
    hid_t dsid = H5Dcreate (_h(id_), name,
                            _h(datatype.id()), _h(memspace.id()),
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dsid < 0) {
        // error
        return QH5Dataset();
    }
    return QH5Dataset(static_cast<QH5id::h5id>(dsid), false);
}
QH5Dataset QH5Group::openDataset(const char *name) const
{
    if (!isDataset(name)) {
        // error
        return QH5Dataset();
    }
    hid_t dsid = H5Dopen(_h(id_), name, H5P_DEFAULT);
    if (dsid < 0) {
        // error
    }
    return QH5Dataset(static_cast<QH5id::h5id>(dsid), false);
}
QVector<QH5Group> QH5Group::subGroups() const
{
    QVector<QH5Group> groups;
    hsize_t n;
    H5Gget_num_objs(_h(id_),&n);
    for(hsize_t i=0; i<n; ++i)
    {
        H5G_obj_t typ = H5Gget_objtype_by_idx(_h(id_),i);
        if (typ==H5G_GROUP)
            groups.push_back(openGroup(objname_by_idx(i)));
    }
    return groups;
}
QVector<QH5Dataset> QH5Group::datasets() const
{
    QVector<QH5Dataset> ds;
    hsize_t n;
    H5Gget_num_objs(_h(id_),&n);
    for(hsize_t i=0; i<n; ++i)
    {
        H5G_obj_t typ = H5Gget_objtype_by_idx(_h(id_),i);
        if (typ==H5G_DATASET) ds.push_back(openDataset(objname_by_idx(i)));
    }
    return ds;
}
QByteArrayList QH5Group::groupNames() const
{
    QByteArrayList names;
    hsize_t n;
    H5Gget_num_objs(_h(id_),&n);
    for(hsize_t i=0; i<n; ++i)
    {
        H5G_obj_t typ = H5Gget_objtype_by_idx(_h(id_),i);
        if (typ==H5G_GROUP) names.push_back(objname_by_idx(i));
    }
    return names;
}

QByteArrayList QH5Group::datasetNames() const
{
    QByteArrayList names;
    hsize_t n;
    H5Gget_num_objs(_h(id_),&n);
    for(hsize_t i=0; i<n; ++i)
    {
        H5G_obj_t typ = H5Gget_objtype_by_idx(_h(id_),i);
        if (typ==H5G_DATASET) names.push_back(objname_by_idx(i));
    }
    return names;
}

QByteArray QH5Group::objname_by_idx(int i) const
{
    QByteArray name;
    ssize_t sz = H5Gget_objname_by_idx(_h(id_),i,NULL,0);
    if (sz) {
        name.resize(sz);
        H5Gget_objname_by_idx(_h(id_),i,name.data(),sz+1);
    }
    return name;
}

