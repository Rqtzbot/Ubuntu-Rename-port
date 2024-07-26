#ifndef PORT_H
#define PORT_H

#include <QDialog>
#include <QStringListModel>
namespace Ui {
class port;
}

class port : public QDialog
{
    Q_OBJECT

public:
    explicit port(QWidget *parent = nullptr);
    ~port();
    QString printshell(std::string data,std::string comm,std::string findstr,int num);

private slots:
    void on_see_kernel_clicked();;

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_change_btn_clicked();

    void on_refresh_clicked();

    void on_udev_btn_clicked();

    void on_clear_clicked();

private:
    Ui::port *ui;
    QChar cr[10];
    int count=0;
    bool flag=false;
    bool usbflag=false;
    bool acmflag=false;
    QChar cr1;
    QChar cr2;
};

#endif // PORT_H
