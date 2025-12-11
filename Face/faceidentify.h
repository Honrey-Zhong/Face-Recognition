#ifndef FACEIDENTIFY_H
#define FACEIDENTIFY_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <opencv.hpp>
#include <QImage>
#include <QBuffer>
#include <QTime>
#include <QList>
#include "qfaceobject.h"
#include "form.h"

#ifdef __cplusplus
extern "C"
{
#pragma pack(push,1)//禁止结构体内存对齐
struct MyData
{
    int staff_id;
    QString staff_name;
    QString staff_division;
    QByteArray staff_image_data;
};//数据包结构体
#pragma pack(pop);//解除禁止，只需要禁止上面的结构体对齐即可
}
#endif
using namespace cv;
using namespace std;

#define IP "192.168.33.208"
#define PORT 8889


QT_BEGIN_NAMESPACE
namespace Ui { class FaceIdentify; }
QT_END_NAMESPACE

class FaceIdentify : public QMainWindow
{
    Q_OBJECT

public:
    FaceIdentify(QWidget *parent = nullptr);
    ~FaceIdentify();
    void timerEvent(QTimerEvent *e);//定时器事件，用于替代while循环，程序的所有功能都在定时器内实现
    void init_client();//tco客户端初始化
    Mat QImageToCvMat(const QImage &image);//把QImage转化为Mat的函数
    QTcpSocket * client; //tcp客户端

    int reg_mask; //注册标志位
    int check_in_mask; //考勤标志位

    QfaceObject * faceobj; //人脸识别功能类
    Form * form; //信息输入表格窗口类
    struct MyData * data; //信息包结构体
    QList<struct MyData *> * data_list; //信息包容器



private slots:
    void on_resg_btn_clicked();

    void on_push_card_btn_clicked();

private:
    Ui::FaceIdentify *ui;

    VideoCapture cap; //摄像头类
    cv::CascadeClassifier cascade; //haar--级联分类器，不知道有什么用
};
#endif // FACEIDENTIFY_H
