#include "qfaceobject.h"

QfaceObject::QfaceObject()
{
    seeta::ModelSetting FDmode("/mnt/hgfs/model/fd_2_00.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting PDmode("/mnt/hgfs/model/pd_2_00_pts5.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting FRmode("/mnt/hgfs/model/fr_2_10.dat",seeta::ModelSetting::CPU,0);

    this->fengineptr = new seeta::FaceEngine(FDmode,PDmode,FRmode);
}

int QfaceObject::face_register(cv::Mat &faceimage)
{
    //把opecv的Mat数据转seetaface数据
    SeetaImageData simage;
    simage.data = faceimage.data;
    simage.width = faceimage.cols;
    simage.height = faceimage.rows;
    simage.channels = faceimage.channels();
    return this->fengineptr->Register(simage);
}

int QfaceObject::face_query(cv::Mat &faceimage)
{
    SeetaImageData simage;
    simage.data = faceimage.data;
    simage.width = faceimage.cols;
    simage.height = faceimage.rows;
    simage.channels = faceimage.channels();
    float similarity = 0;//可信度
    int faceid = this->fengineptr->Query(simage,&similarity);

    qDebug()<<"查询到的人脸ID："<<faceid<<"可信度："<<similarity;
    if(similarity < 0.8)
    {
        return -1;
    }
    return faceid;
}


