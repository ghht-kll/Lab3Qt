#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Client-Server 1");
    this->udpSocket = new QUdpSocket(this);
    this->udpSocket->bind(2424);
    connect(this->udpSocket, SIGNAL(readyRead()), SLOT(SlotProcessDatagrams()));

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("data_message.db");
    if(!db.open())
        qDebug() << "Нет подключения к БД" << db.lastError();
    else qDebug() << "Есть подключение к БД";
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isEmptyLineEdit()
{
    if(this->ui->lineEdit->text() == nullptr)
    {
        QMessageBox::information(nullptr,"Ошибка","Строка пуста");
        return false;
    }
    else return true;
}

void MainWindow::insertNewMessageFromBD(QString str)
{
    QSqlQuery query;
    QString str_insert = "INSERT INTO message_table (data_time, text_message)"
                         "VALUES ('%1', '%2');";
    QString row = str_insert.arg(QDateTime::currentDateTime().toString())
                            .arg(str);
    if(!query.exec(row))
        QMessageBox::information(nullptr,"Ошибка","Что-то не то с БД");
}

void MainWindow::on_pushButton_clicked()
{
    if(this->isEmptyLineEdit())
    {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_DefaultCompiledVersion);
        out << this->ui->lineEdit->text();
        this->udpSocket->writeDatagram(byteArray, QHostAddress::LocalHost, 2425);
        this->insertNewMessageFromBD(this->ui->lineEdit->text());
    }
}

void MainWindow::SlotProcessDatagrams()
{
    QByteArray byteArray;
    do
    {
        byteArray.resize(this->udpSocket->pendingDatagramSize());
        this->udpSocket->readDatagram(byteArray.data(), byteArray.size());
    }
    while (this->udpSocket->hasPendingDatagrams());

    QDataStream in(&byteArray,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);
    QString str;
    in >> str;
    this->ui->plainTextEdit->moveCursor(QTextCursor::End);
    this->ui->plainTextEdit->insertPlainText("Client-Server 2: " + str + "\n" + QDateTime::currentDateTime().toString() + "\n");
    this->insertNewMessageFromBD(str);
}

void MainWindow::on_pushButton_2_clicked()
{
    QSqlQuery query;
    if (query.exec("SELECT * FROM message_table"))
    {
        this->qStandardItemModel = new QStandardItemModel();
        qStandardItemModel->setHorizontalHeaderLabels(QStringList()<<"Time"<<"Message");

        QSqlRecord rec = query.record();
        QString time, message;

        while (query.next())
        {
            time = query.value(rec.indexOf("data_time")).toString();
            message = query.value(rec.indexOf("text_message")).toString();
            qDebug() << "time: " << time << "\nmessage: " << message <<"\n\n";

            QStandardItem* itemCol1(new QStandardItem(time));
            QStandardItem* itemCol2(new QStandardItem(message));

            qStandardItemModel->appendRow(QList<QStandardItem*>()<<itemCol1<<itemCol2);
        }
        this->ui->tableView->setModel(qStandardItemModel);
    }
}

