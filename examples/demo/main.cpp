#include "qthdf5.h"

#include <QFile>

#include <QDebug>

using namespace std;

// list contents under group g
void listAll(const QH5Group& g, int level = 0)
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
        listAll(s, level + 1);
}

int main()
{

    QH5File h5f("TEST.H5");

    // create the file
    if (h5f.open(QIODevice::Truncate))
    {
        QH5Group root = h5f.root();
        if (root.isValid()) {
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
            if (g1.isValid()) {
                g1.write("B",QVector<char>({1,2,3}));
            }
        }
        h5f.close();
    }

    if (h5f.open(QIODevice::ReadOnly))
    {
        listAll(h5f.root());
        h5f.close();
    }

    return 0;
}
