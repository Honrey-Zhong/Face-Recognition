#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->init_database();
    this->init_server();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_database()
{
    this->database = QSqlDatabase::addDatabase("QSQLITE");
    this->database.setDatabaseName("./data.db");
    //打开数据库
    if(!this->database.open())
    {
        qDebug()<<"打开数据库失败";
        exit(-1);
    }
    else {
        qDebug()<<"打开数据库成功";
    }

    this->query = new QSqlQuery;
    if(query->exec("CREATE TABLE face_data(staff_id INTEGER PRIMARY KEY,"
                  "staff_name TEXT,"
                  "staff_division TEXT,"
                  "staff_img MEDIUMBLOB)"))
    {
        qDebug()<<"create table face_data data.ab succeed";
    }
    else {
        qDebug()<<"failed to create table face_data";
        //如果数据表已经存在，这条sql语句会执行失败
    }
    this->model = new QSqlTableModel;
    this->model->setTable("face_data");
    this->ui->tableView->setModel(this->model);
    model->select();
}

void MainWindow::init_server()
{
    //创建服务器对象
    this->server = new QTcpServer;
    //一键启动绑定监听
    if(!this->server->listen(QHostAddress::Any,8889))
    {
        qDebug()<<"服务器绑定或开启监听失败";
        exit(-1);
    }
    //设置ui标志变成绿色
    this->ui->server_state_label->setStyleSheet("border-radius:25px;background-color: rgb(138, 226, 52);");
    //绑定信号与槽，如果有客户端过来连接，则调用对应槽函数
    QObject::connect(this->server,&QTcpServer::newConnection,this,&MainWindow::get_client);

}

void MainWindow::send_data()
{
    //遍历整个数据库，把数据库的所有数据发给客户端
    if(this->query->exec("SELECT * FROM face_data"))
    {
        int staff_id;
        QString staff_name;
        QString staff_division;
        QByteArray img_data;
        while(this->query->next())
        {
            staff_id = this->query->value(0).toInt();
            staff_name = this->query->value(1).toString();
            staff_division = this->query->value(2).toString();
            img_data = this->query->value(3).toByteArray();
            QByteArray array;
            QDataStream out(&array,QIODevice::WriteOnly);//提前设置好序列化出口
            out.setVersion(QDataStream::Qt_4_1);
            //进行序列化输出
            //预留一个位置保存传输的数据大小
            out << (quint32)0;
            out << staff_id << staff_name << staff_division << img_data;
            //回到开头，填充数据大小
            out.device()->seek(0);
            out << (quint32)(array.size() - sizeof(quint32));
            if(this->client->write(array) < 0)
            {
                qDebug()<<"数据发送失败";
            }
            else {
                qDebug()<<"数据发送成功";
                qDebug()<<staff_name;
            }

            this->client->flush();//刷新缓存区
            sleep(8);//延时

        }
    }
    //当程序运行至此，说明数据已经发送完毕
}


void MainWindow::get_client()
{
    //获取前来连接的客户端
    this->client = this->server->nextPendingConnection();
    //绑定信号与槽，如果客户端发来数据，则调用对应接收数据函数
    QObject::connect(this->client,&QTcpSocket::readyRead,[&](){
       //设置反序列化入口
       QDataStream in(this->client);//直接用tcp客户端作为数据源
       in.setVersion(QDataStream::Qt_4_1);
       //如果没有完整读到数据头（获取传输数据大小),则等数据传过来再执行此函数
       if(this->client->bytesAvailable() < sizeof(quint32)) return;
       //读数据大小
       quint32 size;
       in >> size;
       //等待所有数据到达
       while(this->client->bytesAvailable() < size)
       {
           if(!this->client->waitForReadyRead(3000))
           {
               qDebug()<<"数据传输超时";
               return;
           }
       }
       //获取数据包类型
       QString data_type;
       in >> data_type;
       if(data_type == "发送数据")
       {
           struct MyData myData;
           in >> myData.staff_id >> myData.staff_name >> myData.staff_division >> myData.staff_image_data;
           qDebug()<<myData.staff_id;
           qDebug()<<myData.staff_name;
           qDebug()<<myData.staff_division;
           qDebug()<<myData.staff_image_data;
           //准备sql语句
           this->query->prepare("INSERT INTO face_data VALUES(:id,:name,:division,:img)");
           this->query->bindValue(":id",myData.staff_id);
           this->query->bindValue(":name",myData.staff_name);
           this->query->bindValue(":division",myData.staff_division);
           this->query->bindValue(":img",myData.staff_image_data);
           if(!this->query->exec())
           {
               qDebug()<<"插入数据失败";
           }
           else {
               qDebug()<<"插入数据成功";
               this->model->select();
           }
       }
       else if(data_type == "请求数据")
       {
            this->send_data();
       }


    });
    //绑定信号与槽，如果客户端断开连接，则弹出一个对话框
    QObject::connect(this->client,&QTcpSocket::disconnected,[&](){
        QMessageBox::warning(this,"警告","客户端已断开连接");
    });
    qDebug()<<"有客户端连接";
}


void MainWindow::on_find_face_clicked()
{
    //弹出对话框，用于获取员工号，根据员工好进行信息查找
       QString staff_id = QInputDialog::getText(this,"查找","请输入员工号",QLineEdit::Normal);
       //准备sql语句
       this->query->prepare("SELECT * FROM face_data WHERE staff_id = :staff_id");
       this->query->bindValue(":staff_id",staff_id);
       //执行sql语句
       if(this->query->exec())
       {
            if(this->query->next())
            {
                //显示员工信息
                int id = this->query->value(0).toInt();
                QString name = this->query->value(1).toString();
                QString division = this->query->value(2).toString();
                QByteArray img = this->query->value(3).toByteArray();
                this->ui->lineEdit->setText(QString::number(id));
                this->ui->lineEdit_2->setText(name);
                this->ui->lineEdit_3->setText(division);
                QImage show_img;
                show_img.loadFromData(img);
                QPixmap show_mmp = QPixmap::fromImage(show_img);
                this->ui->label->setPixmap(show_mmp);
                //图片标签大小自适应
                this->ui->label->setScaledContents(true);
                //弹出提示框
                QMessageBox::about(this,"结果","查找成功");//因为是根据主键查找，找到的信息只会有一条，或者没有
            }
            else {
                QMessageBox::about(this,"结果","员工不存在");
            }
       }
       else {
          qDebug("查找失败");
       }

}


void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    //双击删除数据
    int row = index.row();//获取被点击的行号
    this->model->removeRow(row);//删除对应行
}
