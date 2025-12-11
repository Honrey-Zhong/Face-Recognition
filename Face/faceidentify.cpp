#include "faceidentify.h"
#include "ui_faceidentify.h"

FaceIdentify::FaceIdentify(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceIdentify)
{
    this->init_client();//tcp客户端初始化
    ui->setupUi(this);
    cap.open(0); //打开摄像头，摄像头位于/dev/video
    startTimer(100);//启动定时器事件，设置定时器间隔为0.1s
    //导入级联分类器文件
    cascade.load("/mnt/hgfs/face_check/opencv452/etc/haarcascades/haarcascade_frontalface_alt2.xml");
    //标志位初始化
    this->reg_mask = 0;
    this->check_in_mask = 0;
    //初始化人脸识别功能类
    this->faceobj = new QfaceObject;
    //初始化输入表单类
    this->form = nullptr;
    //初始化信息包结构体
    this->data = new struct MyData;
    //初始化信息包容器
    this->data_list = new QList<struct MyData *>;

}

FaceIdentify::~FaceIdentify()
{
    qDeleteAll(*data_list);//释放容器内的所有堆空间
    data_list->clear();
    delete ui;
}

void FaceIdentify::timerEvent(QTimerEvent *e)
{
    //采集数据
    Mat srcImage;
    if(cap.grab())
    {
        cap.read(srcImage);//读取一帧数据

    }
    Mat grayImage;
    //转灰度图
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
    //检测人脸数据
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(grayImage,faceRects);
    if(faceRects.size()>0)
    {
        Rect rect = faceRects.at(0);//第一个人脸的矩形框
        rectangle(srcImage,rect,Scalar(0,0,255));
        //移动人脸框（图片--QLabel）
        ui->headpicLb->move(rect.x,rect.y);
    }else
    {
        //把人脸框移动到中心位置
        ui->headpicLb->move(100,60);
    }

    if(srcImage.data == nullptr) return;
    //把opencv里面的Mat格式数据（BGR）转Qt里面的QImage(RGB)
    cvtColor(srcImage,srcImage, COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols, srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap mmp = QPixmap::fromImage(image);
    ui->videoLb->setPixmap(mmp);

    if(this->reg_mask == 1)//注册
    {

        this->reg_mask = 0;
        int id = this->faceobj->face_register(srcImage);
        qDebug()<<"人脸ID:"<<id;
        if(id >= 0)
        {
            //人脸注册成功，收集数据后上传到数据库中
            qDebug()<<"传输人脸数据";
            //收集人脸数据
            QBuffer buffer(&data->staff_image_data);//创建一个缓存区
            buffer.open(QIODevice::WriteOnly);//以只写方式打开缓存区
            image.save(&buffer,"PNG");//通过缓存区把数据保存在结构体内
            //把结构构体的数据序列化后发送给服务器
            QByteArray array;
            QDataStream out(&array,QIODevice::WriteOnly);//提前设置好序列化出口
            out.setVersion(QDataStream::Qt_4_1);
            //进行序列化输出
            //预留一个位置保存传输的数据大小
            out << (quint32)0;
            //添加一个标志位用于区分数据包
            QString type = "发送数据";
            out << type << data->staff_id << data->staff_name << data->staff_division << data->staff_image_data;
            //回到开头，填充数据大小
            out.device()->seek(0);
            out << (quint32)(array.size() - sizeof(quint32));
            //为什么要减，因为这个表示数据大小的位置所占空间是不属于传输数据里的，要做好区分

            if(this->client->write(array) < 0)
            {
                qDebug()<<"数据传输失败";
            }
            else {
                qDebug()<<"数据传输成功";
                qDebug()<<array.size();
                //显示注册信息
                QImage reg_img;//把数据转化成图片
                reg_img.loadFromData(data->staff_image_data);
                QPixmap reg_mmp = QPixmap::fromImage(reg_img);
                this->ui->headLb->setPixmap(reg_mmp);
                //有时候图片远大于标签大小，我们要设置图片和标签的大小适应
                this->ui->headLb->setScaledContents(true);
                this->ui->headLb->setStyleSheet("border-radius:75px;");
                this->ui->staff_id_line->setText(QString::number(data->staff_id));
                this->ui->staff_name_line->setText(data->staff_name);
                this->ui->staff_division_line->setText(data->staff_division);
                QString time_str = QTime::currentTime().toString("hh:mm:ss");
                this->ui->time_line->setText(time_str);
                //提示框显示注册成功
                this->ui->label_2->setText("注册成功");
                this->ui->label->setStyleSheet("background-image: url(:/image/yes.png);");
                //向容器里添加数据
                struct MyData * new_data = new struct MyData;
                new_data->staff_id = data->staff_id;
                new_data->staff_name = data->staff_name;
                new_data->staff_division = data->staff_division;
                new_data->staff_image_data = data->staff_image_data;
                this->data_list->push_back(new_data);

            }
           this->client->flush();//刷新缓存区，冲洗掉脏数据

        }
        else {
            //提示框显示注册失败
            this->ui->label_2->setText("注册失败");
            this->ui->label->setStyleSheet("background-image: url(:/image/no.png);");


        }
    }
    else if(this->check_in_mask == 1)
    {
        this->check_in_mask = 0;
        int face_id;
        face_id = this->faceobj->face_query(srcImage);
        if(face_id >= 0)
        {
            //根据id从容器中获取对应的结构体
            struct MyData * staff_info = this->data_list->at(face_id);
            //显示考勤信息
            QImage reg_img;//把数据转化成图片
            reg_img.loadFromData(staff_info->staff_image_data);
            QPixmap reg_mmp = QPixmap::fromImage(reg_img);
            this->ui->headLb->setPixmap(reg_mmp);
            //有时候图片远大于标签大小，我们要设置图片和标签的大小适应
            this->ui->headLb->setScaledContents(true);
            this->ui->staff_id_line->setText(QString::number(staff_info->staff_id));
            qDebug()<<staff_info->staff_id;
            this->ui->staff_name_line->setText(staff_info->staff_name);
            this->ui->staff_division_line->setText(staff_info->staff_division);
            //设置考勤打卡时间
            QString time_str = QTime::currentTime().toString("hh:mm:ss");
            this->ui->time_line->setText(time_str);
            //提示框显示打卡成功
            this->ui->label_2->setText("打卡成功");
            this->ui->label->setStyleSheet("background-image: url(:/image/yes.png);");


        }
        else {
            //提示框显示打卡失败
            this->ui->label_2->setText("打卡失败");
            this->ui->label->setStyleSheet("background-image: url(:/image/no.png);");

        }


    }
}

void FaceIdentify::init_client()
{
    //创建客户端对象
    this->client = new QTcpSocket;
    //客户端连接到服务器
    this->client->connectToHost(IP,PORT);
    //绑定信号与槽，实现连接成功时，向服务器请求数据
    QObject::connect(this->client,&QTcpSocket::connected,[&](){
        QByteArray array;
        QDataStream out(&array,QIODevice::WriteOnly);//提前设置好序列化出口
        out.setVersion(QDataStream::Qt_4_1);
        //进行序列化输出
        //预留一个位置保存传输的数据大小
        out << (quint32)0;
        //添加一个标志位用于区分数据包
        QString type = "请求数据";
        out << type;
        //回到开头，填充数据大小
        out.device()->seek(0);
        out << (quint32)(array.size() - sizeof(quint32));
        if(this->client->write(array) < 0)
        {
            qDebug()<<"数据请求失败";
        }
        else {
            qDebug()<<"数据请求成功";
        }
        this->client->flush();//刷新缓存区
    });
    //绑定信号与槽，实现接收从服务器发来的数据
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
        struct MyData * myData = new struct MyData;
        //用结构体接收数据
        in >> myData->staff_id >> myData->staff_name >> myData->staff_division >> myData->staff_image_data;
//        qDebug()<<myData->staff_id;
//        qDebug()<<myData->staff_name;
//        qDebug()<<myData->staff_division;
//        qDebug()<<myData->staff_image_data;
        //注册人脸信息
        //通过QImage中转将QByteArray转化为opencv mat数据
        QImage image;
        image.loadFromData(myData->staff_image_data);
        Mat src_img = QImageToCvMat(image);
        //注册花费的时间过长导致数据接收不完整
        int id = this->faceobj->face_register(src_img);
        qDebug()<<"人脸ID:"<<id;
        if(id < 0)
        {
            qDebug()<<"注册失败";
        }
        else {
            qDebug()<<"注册成功";
            //把有效数据放进容器中
            this->data_list->push_back(myData);
        }
        qDebug()<<myData->staff_name;
    });
}

Mat FaceIdentify::QImageToCvMat(const QImage &image)
{
    switch (image.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            cv::Mat mat(image.height(), image.width(), CV_8UC4,
                       (void*)image.constBits(), image.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_BGRA2BGR);
            return result;
        }

        case QImage::Format_RGB888: {
            cv::Mat mat(image.height(), image.width(), CV_8UC3,
                       (void*)image.constBits(), image.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
            return result;
        }

        case QImage::Format_Grayscale8: {
            cv::Mat mat(image.height(), image.width(), CV_8UC1,
                       (void*)image.constBits(), image.bytesPerLine());
            return mat.clone();
        }

        default: {
            // 转换为支持的格式
            QImage converted = image.convertToFormat(QImage::Format_RGB888);
            cv::Mat mat(converted.height(), converted.width(), CV_8UC3,
                       (void*)converted.constBits(), converted.bytesPerLine());
            cv::Mat result;
            cv::cvtColor(mat, result, cv::COLOR_RGB2BGR);
            return result;
        }
        }
}


void FaceIdentify::on_resg_btn_clicked()
{
    //创建一个新窗口，用于收集注册信息
    if(this->form == nullptr)
    {
        this->form = new Form(this,data->staff_id,data->staff_name,data->staff_division);
        this->form->move(480,0);//移动窗口
        //绑定信号与槽
        QObject::connect(this->form,&Form::has_submited,[&](){
            qDebug()<<data->staff_id << data->staff_name << data->staff_division;
            if(data->staff_image_data.isEmpty())
            {
                qDebug()<<"图片资源暂未获取";
            }
            this->form->close();//关闭窗口
            //当获取到注册信息时，注册标志位设置为1
            this->reg_mask = 1;
        });
    }
    this->form->show();

}

void FaceIdentify::on_push_card_btn_clicked()
{
    this->check_in_mask = 1;
}
