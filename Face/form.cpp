#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent,int & set_staff_code,QString & set_staff_name,QString & set_staff_division) :
    QWidget(parent),
    staff_code(set_staff_code),
    staff_name(set_staff_name),
    staff_division(set_staff_division),
    ui(new Ui::Form)
{
    ui->setupUi(this);
}

Form::~Form()
{
    delete ui;
}


void Form::on_cancle_btn_clicked()
{
    this->close();
}

void Form::on_confirm_btn_clicked()
{
    //获取单行文本框的信息
    this->staff_code = this->ui->staff_code_line->text().toInt();
    this->staff_name = this->ui->staff_name_line->text();
    this->staff_division = this->ui->staff_division_line->text();
    //staff_id如果没有输入默认为0，不需要考虑没有输入staff_id的问题
    if(this->staff_name.isEmpty()||this->staff_division.isEmpty())
    {
        QMessageBox::warning(this,"警告","输入的信息不完整");
        return;
    }
    //发送信号
    emit this->has_submited();
}
