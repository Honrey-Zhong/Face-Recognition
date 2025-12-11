#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QMessageBox>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent,int & set_staff_code,QString & set_staff_name,QString & set_staff_division);
    ~Form();
    int & staff_code;
    QString & staff_name;
    QString & staff_division;
signals:
    void has_submited(); //自定义信号
private slots:
    void on_cancle_btn_clicked();

    void on_confirm_btn_clicked();

private:
    Ui::Form *ui;
};

#endif // FORM_H
