#include "port.h"
#include "ui_port.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <iostream>
#include <fstream>
#include <sstream>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QFileDialog>
#include <cstdio>
#include <string>
#include <array>
//构造函数
port::port(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::port)
{
    ui->setupUi(this);
    setWindowTitle("Ubuntu串口重映射助手");
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    //遍历串口信息列表并输出串口号
    foreach (const QSerialPortInfo &serialPortInfo, serialPorts) {
//        qDebug() << "Serial Port: " << serialPortInfo.portName();
        count++;
        ui->serial_list->addItem(serialPortInfo.portName());
    }
    setWindowIcon(QIcon(":/port"));
    setMinimumWidth(528);
    setMinimumHeight(700);
    ui->num_edit->setText(QString::number(count));
    ui->usb_check->setCheckState(Qt::Checked);
    ui->acm_check->setCheckState(Qt::Unchecked);
    ui->udev_btn->setDisabled(true);

    connect(ui->usb_check,&QCheckBox::stateChanged,this,[=](int state)
    {
        if(state>1)
            ui->acm_check->setCheckState(Qt::Unchecked);
    });
    connect(ui->acm_check,&QCheckBox::stateChanged,this,[=](int state)
    {
        if(state>1)
            ui->usb_check->setCheckState(Qt::Unchecked);
    });
    connect(ui->serial_list,&QComboBox::currentTextChanged,this,[=]()
    {
        ui->set_portname->clear();
        ui->rule_edit->clear();
        ui->KERENEL->clear();
    });
}
//析构函数（细狗）
port::~port()
{
    delete ui;
}
//将shell命令输出到管道并按规则打印
QString port::printshell(std::string data,std::string comm,std::string findstr,int num)
{

    std::array<char,128> buffer;
    std::string res;
    //字符串拼接
    std::stringstream str;
    str<<comm<<data<<" | grep '"<<findstr<<"' | awk '{print $"<<num<<"}'";
    qDebug()<<str.str().c_str();
    //创建一个文件对象指针
    FILE* pipe = popen(str.str().c_str(),"r");
    if (!pipe) {
            std::cerr << "popen() failed!" << std::endl;
        }
        //fgets函数直到有一个换行符或文件末尾才终止
        int temp = 0;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {

            temp++;
            //第二个为需要的那一个
            if(temp==2)
            {
                ui->KERENEL->setText(buffer.data());
                qDebug()<<buffer.data();
            }

            res+=buffer.data();
        }
        auto returnpope = pclose(pipe);
        return QString::fromStdString(res);
}
//判断是USB还是ACM并查询串口内核编号
void port::on_see_kernel_clicked()
{
  for(int i=0;i<ui->serial_list->currentText().length();i++)
  {
     if(ui->serial_list->currentText()[i]=="U"&&ui->serial_list->currentText()[i+1]=="S"&&ui->serial_list->currentText()[i+2]=="B")
     {
         cr1=ui->serial_list->currentText()[i+3];
         usbflag=true;
     }
     if(ui->serial_list->currentText()[i]=="A"&&ui->serial_list->currentText()[i+1]=="C"&&ui->serial_list->currentText()[i+2]=="M")
     {
         cr2=ui->serial_list->currentText()[i+3];
         acmflag=true;
     }

  }
   if(usbflag)
    printshell(QString(cr1).toStdString(),"udevadm info --attribute-walk --name=/dev/ttyUSB","KERNELS",1);
   else if(acmflag)
    printshell(QString(cr2).toStdString(),"udevadm info --attribute-walk --name=/dev/ttyUSB","KERNELS",1);

}
//输出串口内核命令kernels
void port::on_pushButton_2_clicked()
{
    if(ui->KERENEL->text()==""||ui->set_portname->text() == "")
    {
        ui->textEdit->append("串口内核或串口别名不能为空..................");
    }
    else {
        //除去最后那个换行符
        QString aimker = "";
        for (int i = 0;i<ui->KERENEL->text().size();i++) {
            if(ui->KERENEL->text()[i]=="\n")
                break;
            aimker+=ui->KERENEL->text()[i];
        }
        //串口内核命令
        QString com=aimker+", MODE:=\"0777\", GROUP:=\"dialout\", SYMLINK+=\"" +ui->set_portname->text()+"\"";
        ui->rule_edit->setText(com);
        // QProcess* pro = new QProcess(this);
        // QString command = "gnome-terminal --geometry=80x18+0+732";
        // pro->start(command);
        if(usbflag)
        {
            ui->textEdit->insertHtml(QString("<strong><font color= 'red' >%1</font> </strong> ").arg("/dev/ttyUSB"+QString(cr1)));
            ui->textEdit->insertHtml(QString("<font color= 'blue' >%1</font><hr> ").arg(com));
        }

        else if(acmflag)  {
            ui->textEdit->insertHtml(QString("<strong><font color= 'red' >%1</font> </strong>").arg("/dev/ttyACM"+QString(cr2)));
            ui->textEdit->insertHtml(QString("<font color= 'blue' >%1</font><hr> ").arg(com));
        }
    }
}
//查询串口修改
void port::on_pushButton_3_clicked()
{

   if(ui->usb_check->isChecked())
   {
    QString com = printshell(QString("").toStdString(),"ls -l /dev","ttyUSB",0);
    //富文本
    ui->textEdit->insertHtml(QString("<font color= 'green' >%1</font><hr>").arg(com));
    
   }
   else if(ui->acm_check->isChecked())
   {
       QString com = printshell(QString("").toStdString(),"ls -l /dev","ttyACM",0);
       ui->textEdit->insertHtml(QString("<font color= 'green' >%1</font><hr>").arg(com));
   }

}
//打开usb.rules文件修改
void port::on_change_btn_clicked()
{
    if(ui->password->text()=="")
    {
        ui->textEdit->insertPlainText("请输入管理员密码...............\n");
    }
    else {
        //使用&使其在后台运行不妨碍主线程
        std::string command = "echo '"+ui->password->text().toStdString()+"' | sudo -S gedit /etc/udev/rules.d/usb.rules &";
        system(command.c_str());
        ui->udev_btn->setDisabled(false);
    }

}
//刷新串口
void port::on_refresh_clicked()
{
    count = 0 ;
    ui->serial_list->clear();
    QList<QSerialPortInfo> serialPorts = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &serialPortInfo, serialPorts) {
//        qDebug() << "Serial Port: " << serialPortInfo.portName();
        count++;
        ui->serial_list->addItem(serialPortInfo.portName());
    }
    ui->num_edit->setText(QString::number(count));
}
//重启udev规则
void port::on_udev_btn_clicked()
{
    if(ui->password->text()=="")
    {
        ui->textEdit->insertPlainText("请输入管理员密码.............\n");
    }
    else {
        std::string udev = "echo '"+ui->password->text().toStdString()+"' | sudo -S udevadm trigger";
        system(udev.c_str());
    }

}
//清空数据
void port::on_clear_clicked()
{
    ui->textEdit->clear();
}
