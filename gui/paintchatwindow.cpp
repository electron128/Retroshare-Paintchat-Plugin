#include "paintchatwindow.h"
#include "ui_paintchatwindow.h"

#include <QClipboard>

#include <retroshare/rspeers.h>

#include "interface/paintchatservice.h"
PaintChatWindow::PaintChatWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PaintChatWindow)
{
    ui->setupUi(this);
    connect(ui->paintWidget,SIGNAL(haveUpdate()),SLOT(on_haveUpdate()));

    ui->pushButton1px->setChecked(true);
    ui->pushButtonBlack->setChecked(true);

    ui->paintWidget->color=Qt::black;
    ui->paintWidget->penWidth=1;

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),SLOT(on_timer()));
    timer->start(500);

    ui->label->setText(QString::fromStdString(std::string("no Peer set")));

    QIcon icon ;
    icon.addPixmap(QPixmap(":/images/colors.png"));
    setWindowIcon(icon);
}

PaintChatWindow::~PaintChatWindow()
{
    delete ui;
}

void PaintChatWindow::setPeerId(std::string peerId){
    this->peerId=peerId;
    ui->label->setText(QString::fromStdString(std::string("Paintchat with ")+rsPeers->getPeerName(peerId)));
    setWindowTitle(QString::fromStdString(rsPeers->getPeerName(peerId)+" - PaintChat"));

    ImageResource res;
    res.fromQImage(ui->paintWidget->getImage());
    paintChatService->init(peerId,res);
}

void PaintChatWindow::on_haveUpdate(){
    std::cerr<<"PaintChatWindow::on_haveUpdate()"<<std::endl;
    updateImage();
}

void PaintChatWindow::on_timer(){
    if(paintChatService->haveUpdate(peerId)){
        std::cerr<<"PaintChatWindow::on_timer(): paintChatService->haveUpdate(peerId) returned true"<<std::endl;
        updateImage();
    }
}

void PaintChatWindow::updateImage(){
    if(paintChatService->receivedInit(peerId)){
        ui->paintWidget->fillImage(Qt::white);
        ImageResource res;
        res.fromQImage(ui->paintWidget->getImage());
        paintChatService->init(peerId,res);
    }else{
        ImageResource res;
        res.fromQImage(ui->paintWidget->getImage());
        ui->paintWidget->setImage(paintChatService->update(peerId,res).toQImage());
    }
}

void PaintChatWindow::resetPenButtons(){
    ui->pushButton1px->setChecked(false);
    ui->pushButton4px->setChecked(false);
    ui->pushButton8px->setChecked(false);
}

void PaintChatWindow::on_pushButtonBlack_clicked()
{
    ui->pushButtonWhite->setChecked(false);
    ui->pushButtonBlack->setChecked(true);
    ui->paintWidget->color=Qt::black;
}

void PaintChatWindow::on_pushButtonWhite_clicked()
{
    ui->pushButtonWhite->setChecked(true);
    ui->pushButtonBlack->setChecked(false);
    ui->paintWidget->color=Qt::white;
}

void PaintChatWindow::on_pushButton1px_clicked()
{
    resetPenButtons();
    ui->pushButton1px->setChecked(true);
    ui->paintWidget->penWidth=1;
}

void PaintChatWindow::on_pushButton4px_clicked()
{
    resetPenButtons();
    ui->pushButton4px->setChecked(true);
    ui->paintWidget->penWidth=4;
}

void PaintChatWindow::on_pushButton8px_clicked()
{
    resetPenButtons();
    ui->pushButton8px->setChecked(true);
    ui->paintWidget->penWidth=8;
}

void PaintChatWindow::on_pushButtonClear_clicked()
{
    //käse tut ned, vermutlich wegen gleichen timestamps
    /*
    // not the best way to reset the image, because it causes the entire image to be transmitted
    // first overwrite with black
    ui->paintWidget->fillImage(Qt::black);
    updateImage();
    // then with white
    ui->paintWidget->fillImage(Qt::white);
    updateImage();
    */

    // andere Lösung:
    // sendet viele bytes, weil resource unkomprimiert ist
    // tut nicht, vielleicht zu große items?

    ui->paintWidget->fillImage(Qt::white);
    ImageResource res;
    res.fromQImage(ui->paintWidget->getImage());
    paintChatService->init(peerId,res);
    paintChatService->sendInit(peerId,res);


}

void PaintChatWindow::on_pushButtonCopy_clicked()
{
    QApplication::clipboard()->setImage(ui->paintWidget->getImage());
}
