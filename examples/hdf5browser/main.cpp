#include "h5browserwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    H5BrowserWidget w;
    w.show();
    return a.exec();
}

/*! \example examples/hdf5browser/main.cpp
 *
 * This example shows how to use QtHDF5 and the Qt model/view architecture
 * to construct a simple HDF5 browser application.
 *
 * It defines the class QH5FileModel which represents the HDF5 file
 * as a Qt item model.
 *
 * The QH5FileModel is used in combination with a QtTreeView
 * to display the information stored in HDF5 files and thus
 * create a simple HDF5 file browser.
 *
 * \image html qthdf5-file-browser.png
 *
 */
