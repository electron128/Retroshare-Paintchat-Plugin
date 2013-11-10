#include "paintchatpopupchatdialog.h"

PaintChatPopupChatDialog::PaintChatPopupChatDialog(QWidget *parent)
    : PopupChatDialog(parent)
{
    paintChatWindow=new PaintChatWindow();

    paintChatWindow->hide();

    paintChatWindowToggleButton = new QToolButton ;
    paintChatWindowToggleButton->setMinimumSize(QSize(28,28)) ;
    paintChatWindowToggleButton->setMaximumSize(QSize(28,28)) ;
    paintChatWindowToggleButton->setText(QString()) ;
    paintChatWindowToggleButton->setToolTip(tr("PaintChat"));
    paintChatWindowToggleButton->setAutoRaise(true);
    paintChatWindowToggleButton->setIconSize(QSize(24,24));

    QIcon icon ;
    icon.addPixmap(QPixmap(":/images/colors.png"));
    paintChatWindowToggleButton->setIcon(icon);

    connect(paintChatWindowToggleButton,SIGNAL(clicked()),this,SLOT(togglePaintChatWindow()));

    addChatBarWidget(paintChatWindowToggleButton);
}

void PaintChatPopupChatDialog::togglePaintChatWindow(){
    if(paintChatWindow->isHidden()){
        paintChatWindow->show();
    }else{
        paintChatWindow->hide();
    }
}

void PaintChatPopupChatDialog::init(const std::string &peerId, const QString &title){
    PopupChatDialog::init(peerId,title);
    paintChatWindow->setPeerId(getPeerId());
}
