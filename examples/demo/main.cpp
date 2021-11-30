#include "qthdf5.h"

#include <QDebug>

/*! \example examples/demo/main.cpp
 *
 * This is a basic example of how to use QtHDF5.
 *
 * A HDF5 is created and some groups, attributes and datasets are written to it.
 *
 * Then the file is re-opened in read-only mode and the contents are listed.
 *
 */

// list contents under group g
void list(const QH5Group& g, int level = 0);

int main()
{
    QH5File h5f("TEST.H5");

    bool creation_order = true;

    try {

    // create the file
    if (h5f.open(QIODevice::Truncate))
    {
        // get the root group
        QH5Group root = h5f.root();
        // write an int
        root.write("B",1);
        // write some UTF8 strings
        QStringList L;
        L << QString("Γιώργος") << QString("Γιάννης");
        root.write("A",L);
        // create some groups / sub-groups
        QH5Group g0 = root.createGroup("G0",creation_order);
        g0.writeAttribute("name",QString("G0"));
        g0.writeAttribute("version",3);
        QH5Group g2 = g0.createGroup("G2",creation_order);
        QH5Group g1 = g0.createGroup("G1",creation_order);
        QH5Group g3 = g2.createGroup("G3",creation_order);
        QH5Group g4 = g2.createGroup("G4",creation_order);
        // write a char array
        g3.write("B",QVector<char>({1,2,3}));
        // close the file
        h5f.close();
    }

    }
    catch (const h5exception& e)
    {
        qDebug() << e.what();
        return -1;
    }

    // open the file and list contents
    if (h5f.open(QIODevice::ReadOnly))
    {
        list(h5f.root());
        h5f.close();
    }

    return 0;
}

void listAttributes(const QH5Node& n, int level)
{
    Q_UNUSED(level);

    foreach(const QByteArray& name, n.attributeNames()) {
        QH5Datatype dt = n.attributeType(name);
        QH5Datatype::Class cls = dt.getClass();
        if (cls==QH5Datatype::FLOAT) {
            double v;
            n.readAttribute(name,v);
            qDebug() << "  Attribute" << name << "=" << v;
        } else if (cls==QH5Datatype::INTEGER) {
            int v;
            n.readAttribute(name,v);
            qDebug() << "  Attribute" << name << "=" << v;
        } else if (cls==QH5Datatype::STRING) {
            QString v;
            n.readAttribute(name,v);
            qDebug() << "  Attribute" << name << "=" << v;
        }
        else {
            qDebug() << "  Attribute" << name << ": Uncknown datatype";
        }
    }
}


void listDatasets(const QH5Group& g, int level)
{
    foreach(const QByteArray& name, g.datasetNames()) {
        qDebug() << "Dataset " << name;
        QH5Dataset ds = g.openDataset(name);
        listAttributes(ds,level);
        QH5Datatype dt = ds.datatype();
        QH5Datatype::Class cls = dt.getClass();
        if (cls==QH5Datatype::FLOAT) {
            QVector<double> v;
            ds.read(v);
            qDebug() << v;
        } else if (cls==QH5Datatype::INTEGER) {
            QVector<int> v;
            ds.read(v);
            qDebug() << v;
        } else if (cls==QH5Datatype::STRING) {
            QStringList v;
            ds.read(v);
            qDebug() << v;
        }
        else {
            qDebug() << "Unckown datatype";
        }
    }
}

void list(const QH5Group& g, int level)
{
    qDebug() << g.name();
    listAttributes(g,level);
    listDatasets(g,level);

    foreach(const QH5Group& s, g.subGroups(true))
        list(s, level + 1);
}
