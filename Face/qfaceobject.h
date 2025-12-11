#ifndef QFACEOBJECT_H
#define QFACEOBJECT_H

#include <QObject>
#include <QDebug>
#include <opencv.hpp>
#include "FaceEngine.h"

//人脸数据存储 + 人脸数据检测  + 人脸识别
class QfaceObject
{
public:
    QfaceObject();

public slots:
    int face_register(cv::Mat & faceimage);//人脸注册
    int face_query(cv::Mat & faceimage); //查询人脸返回ID
private:
    seeta::FaceEngine * fengineptr;

};

#endif // QFACEOBJECT_H
