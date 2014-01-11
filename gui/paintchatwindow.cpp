#include "paintchatwindow.h"
#include "ui_paintchatwindow.h"

#include <QClipboard>
#include <QColorDialog>
#include <QBuffer>
#include <QPainter>
#include <QMessageBox>

#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>

const int MAX_LOBBY_MSG_SIZE = 6000;

#include "interface/paintchatservice.h"
PaintChatWindow::PaintChatWindow(QWidget *parent, std::string peerId, ChatWidget *chatWidget) :
    QMainWindow(parent), peerId(peerId), chatType(ChatWidget::CHATTYPE_UNKNOWN), chatWidget(chatWidget),
    ui(new Ui::PaintChatWindow)
{
    ui->setupUi(this);
    connect(ui->paintWidget,SIGNAL(haveUpdate()),SLOT(on_haveUpdate()));
    connect(ui->penWidthSpinBox,SIGNAL(valueChanged(int)),this,SLOT(penChanged()));

    ui->pushButton1px->setChecked(true);
    ui->pushButtonPen->setChecked(true);

    ui->paintWidget->color=Qt::black;
    ui->paintWidget->penWidth=1;

    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),SLOT(on_timer()));
    timer->start(500);

    QIcon icon ;
    icon.addPixmap(QPixmap(":/images/colors.png"));
    setWindowIcon(icon);
    
    QPixmap pix(24, 24);
    pix.fill(currentColor);
    ui->pushButtonPrimaryColor->setIcon(pix);

    chatType = chatWidget->chatType();
    if(chatType == ChatWidget::CHATTYPE_PRIVATE)
    {
        ui->label->setText(QString::fromStdString(std::string("Paintchat with ")+rsPeers->getPeerName(peerId)));
        setWindowTitle(QString::fromStdString(rsPeers->getPeerName(peerId)+" - PaintChat"));

        ImageResource res;
        res.fromQImage(ui->paintWidget->getImage());
        paintChatService->init(peerId,res);

        ui->progressBar->hide();
    }
    if(chatType == ChatWidget::CHATTYPE_LOBBY)
    {
        ui->progressBar->setRange(0,100);
        ui->progressBar->setValue(0);

        ChatLobbyId id;
        rsMsgs->isLobbyId(peerId, id);
        std::string nick, lobby;
        std::list<ChatLobbyInfo> cil;
        rsMsgs->getChatLobbyList(cil);
        std::list<ChatLobbyInfo>::iterator it;
        for(it = cil.begin(); it != cil.end(); it++)
        {
            ChatLobbyInfo ci = *it;
            if(ci.lobby_id == id)
            {
                nick = ci.nick_name;
                lobby = ci.lobby_name;
                break;
            }
        }
        std::string label = nick + "@" + lobby;
        ui->label->setText(QString::fromStdString(label));
        setWindowTitle(QString::fromStdString(label + " - PaintChat"));
    }
}

PaintChatWindow::~PaintChatWindow()
{
    delete ui;
}

void PaintChatWindow::on_haveUpdate(){
    std::cerr<<"PaintChatWindow::on_haveUpdate()"<<std::endl;

    if(chatType == ChatWidget::CHATTYPE_PRIVATE)
    {
        updateImage();
    }
    if(chatType == ChatWidget::CHATTYPE_LOBBY)
    {
        QImage i = ui->paintWidget->getImage();
        QPainter p(&i);
        p.setPen(Qt::black);
        p.setBrush(Qt::NoBrush);
        p.drawRect(0, 0, 302, 102);
        ui->paintWidget->setImage(i);

        QImage img = ui->paintWidget->getImage().copy(1, 1, 301, 101);
        std::string html = imgToHtmlString(img);
        ui->progressBar->setValue((html.size()*100)/MAX_LOBBY_MSG_SIZE);

        if(html.size()>MAX_LOBBY_MSG_SIZE)
        {
            ui->progressBar->setValue(100);
        }
    }
}

void PaintChatWindow::on_timer(){
    if((chatType == ChatWidget::CHATTYPE_PRIVATE)&&(paintChatService->haveUpdate(peerId))){
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

void PaintChatWindow::on_pushButtonPrimaryColor_clicked()
{
  bool ok;
	QRgb color = QColorDialog::getRgba( Qt::black , &ok, window());
	if (ok) {
		ui->paintWidget->color=QColor(color);
		currentColor = QColor(color);
		colorChanged();
	}
}

void PaintChatWindow::colorChanged()
{
	QPixmap pix(24, 24);
	pix.fill(currentColor);
	ui->pushButtonPrimaryColor->setIcon(pix);
}

void PaintChatWindow::on_pushButtonPen_clicked()
{
    ui->pushButtonWhite->setChecked(false);
    ui->pushButtonPen->setChecked(true);

		ui->paintWidget->color= currentColor; 
}

void PaintChatWindow::on_pushButtonWhite_clicked()
{
    ui->pushButtonWhite->setChecked(true);
    ui->pushButtonPen->setChecked(false);
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

std::string PaintChatWindow::imgToHtmlString(QImage img)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return std::string(std::string("<img src=\"data:image/png;base64,") + ba.toBase64().data() + "\"/>");
}

void PaintChatWindow::on_pushButtonSend_clicked()
{
    std::string html;
    if(chatType == ChatWidget::CHATTYPE_PRIVATE)
    {
        html = imgToHtmlString(ui->paintWidget->getImage());
    }
    if(chatType == ChatWidget::CHATTYPE_LOBBY)
    {
        QImage img = ui->paintWidget->getImage().copy(1, 1, 301, 101);
        html = imgToHtmlString(img);
        if(html.size() > MAX_LOBBY_MSG_SIZE)
        {
            QMessageBox msgBox;
            msgBox.setText(tr("The image is to big to send it in a Chatlobby. Try to remove Details. The Progressbar below the image shows you the image size."));
            msgBox.exec();
            return;
        }
    }
    rsMsgs->sendPrivateChat(peerId, std::wstring(html.begin(), html.end()));
    chatWidget->addChatMsg(false, QString::fromStdString(rsPeers->getPeerName(rsPeers->getOwnId())),
                           QDateTime::currentDateTime(), QDateTime::currentDateTime(),
                           QString::fromStdString(html), ChatWidget::MSGTYPE_NORMAL );
}

void PaintChatWindow::penChanged()
{
    resetPenButtons();

    int width = ui->penWidthSpinBox->value();
    ui->paintWidget->penWidth= width;
}
