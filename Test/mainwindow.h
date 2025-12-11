#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QDebug>

#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDataStream>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>

#ifdef __cplusplus
extern "C"
{
#include <unistd.h>
#pragma pack(push,1)//禁止结构体内存对齐
struct MyData
{
    int staff_id;
    QString staff_name;
    QString staff_division;
    QByteArray staff_image_data;
};//数据包结构体
#pragma pack(pop)//解除禁止，只需要禁止上面的结构体对齐即可
}
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init_database();//初始化数据库
    void init_server();//初始化tcp服务器
    void send_data(); //发送数据给客户端的函数
    QSqlDatabase database;//数据库对象
    QSqlQuery * query;//sql语句对象
    QSqlTableModel * model;//数据模型对象

    QTcpServer * server; //tcp服务器类
    QTcpSocket * client; //tcp客户端类

public slots:
    void get_client();

private slots:
    void on_find_face_clicked();



    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
