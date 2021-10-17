#include "qthdf5.h"

#include <QDebug>

// list contents under group g
void list(const QH5Group& g, int level = 0);

int main()
{
    QH5File h5f("TEST.H5");

    // create the file
    if (h5f.open(QIODevice::Truncate))
    {
        // get the root group
        QH5Group root = h5f.root();
        // write an int
        root.write("A",1);
        // write some UTF8 strings
        QStringList L;
        L << QString("Γιώργος") << QString("Γιάννης");
        root.write("B",L);
        // create some groups / sub-groups
        QH5Group g1 = root.createGroup("G1");
        QH5Group g2 = root.createGroup("G2");
        QH5Group g3 = g2.createGroup("G3");
        // write a char array
        g3.write("B",QVector<char>({1,2,3}));
        // close the file
        h5f.close();
    }

    // open the file and list contents
    if (h5f.open(QIODevice::ReadOnly))
    {
        list(h5f.root());
        h5f.close();
    }

    return 0;
}

void list(const QH5Group& g, int level)
{
    qDebug() << g.name();
    foreach(const QByteArray& name, g.datasetNames()) {
        qDebug() << "Dataset " << name;
        QH5Dataset ds = g.openDataset(name);
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
    foreach(const QH5Group& s, g.subGroups())
        list(s, level + 1);
}
