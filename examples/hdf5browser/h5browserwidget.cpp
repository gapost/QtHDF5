#include "h5browserwidget.h"
#include "ui_h5browserwidget.h"

#include "qh5filemodel.h"

#include <QFileDialog>

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
    ui->edtField->setPlainText(model_->toString(index));

}
