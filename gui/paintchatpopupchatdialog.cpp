#include "paintchatpopupchatdialog.h"

PaintChatPopupChatDialog::PaintChatPopupChatDialog(ChatWidget *chatWidget)
    : QObject(), ChatWidgetHolder(chatWidget)
{
    paintChatWindow=new PaintChatWindow();
    paintChatWindow->setPeerId(mChatWidget->getPeerId());

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

    mChatWidget->addChatBarWidget(paintChatWindowToggleButton) ;
}

void PaintChatPopupChatDialog::togglePaintChatWindow(){
    if(paintChatWindow->isHidden()){
        paintChatWindow->show();
    }else{
        paintChatWindow->hide();
    }
}
