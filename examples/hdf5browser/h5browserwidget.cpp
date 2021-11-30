#include "h5browserwidget.h"
#include "ui_h5browserwidget.h"

#include "qh5filemodel.h"

#include <QFileDialog>
#include <QTextStream>

template<typename T>
QTextStream& operator<<(QTextStream& s, const QVector<T>& v)
{
    if (v.size()>1) {
        s << "( ";
        for (int i=0; i<v.size()-1; i++) s << v[i] << ", ";
        s << v[v.size()-1] << " )";
    } else s << v[0];
    return s;
}

QTextStream& operator<<(QTextStream& s, const QStringList& v)
{
    if (v.size()>1) {
        s << "( ";
        foreach(const QString& t, v) s << "\"" << t << "\", ";
        s << " )";
    } else s << v[0];
    return s;
}

H5BrowserWidget::H5BrowserWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::H5BrowserWidget)
{
    ui->setupUi(this);

    model_ = new QH5FileModel(this);

    connect(ui->btOpen, SIGNAL(pressed()), this, SLOT(openPressed()));
    connect(ui->treeView, &QTreeView::clicked, this, &H5BrowserWidget::on_treeView_activated);
}

H5BrowserWidget::~H5BrowserWidget()
{
    delete ui;
}

void H5BrowserWidget::openPressed()
{
    QString fname = QFileDialog::getOpenFileName(this,
                                                 "Select HDF5 file");

    if (!fname.isEmpty() && QH5File::isHDF5(fname)) {
        ui->treeView->setModel(0);
        model_ ->setFile(fname);
        ui->treeView->setModel(model_);
        ui->edtFileName->setText(fname);
    }

}


void H5BrowserWidget::on_treeView_activated(const QModelIndex &index)
{
    QString S;
    QTextStream str(&S);
    QH5Node node = model_->h5node(index);


    if (node.isGroup()) {
        str << node.name() << ": Group";
    } else if (node.isDataset())
    {
        QH5Dataset ds = node.toDataset();
        str << node.name() << ": Dataset" << endl;
        QH5Datatype dt = ds.datatype();
        QH5Datatype::Class cls = dt.getClass();
        if (cls==QH5Datatype::FLOAT) {
            QVector<double> v;
            ds.read(v);
            str << "Type: FLOAT" << endl;
            str << "Size: " << v.size() << endl;
            str << "Data: " << v;
        } else if (cls==QH5Datatype::INTEGER) {
            QVector<int> v;
            ds.read(v);
            str << "Type: INT" << endl;
            str << "Size: " << v.size() << endl;
            str << "Data: " << v;
        } else if (cls==QH5Datatype::STRING) {
            QStringList v;
            ds.read(v);
            str << "Type: STRING" << endl;
            str << "Size: " << v.size() << endl;
            str << "Data: " << v;
        }
        else {
            str << "Type: Unknown" << endl;
        }

    }
    else str << "Unknown Object";

    str.flush();

    ui->edtField->setPlainText(S);



}
