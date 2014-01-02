#ifndef PAINTCHATPOPUPCHATDIALOG_H
#define PAINTCHATPOPUPCHATDIALOG_H

#include <gui/chat/ChatWidget.h>
#include <QToolButton>
#include "paintchatwindow.h"

class PaintChatPopupChatDialog : public QObject, public ChatWidgetHolder
{
    Q_OBJECT
public:
    PaintChatPopupChatDialog(ChatWidget *chatWidget);

private slots:
    void togglePaintChatWindow();

protected:

private:
    QToolButton *paintChatWindowToggleButton;
    PaintChatWindow *paintChatWindow;
};

#endif // PAINTCHATPOPUPCHATDIALOG_H
