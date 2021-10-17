#ifndef H5BROWSERWIDGET_H
#define H5BROWSERWIDGET_H

#include <QWidget>

class QH5FileModel;

QT_BEGIN_NAMESPACE
namespace Ui { class H5BrowserWidget; }
QT_END_NAMESPACE

class H5BrowserWidget : public QWidget
{
    Q_OBJECT

public:
    H5BrowserWidget(QWidget *parent = nullptr);
    ~H5BrowserWidget();

public slots:
    void openPressed();

private slots:
    void on_treeView_activated(const QModelIndex &index);

private:
    Ui::H5BrowserWidget *ui;

    QH5FileModel* model_;


};
#endif // H5BROWSERWIDGET_H
