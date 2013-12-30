#ifndef PAINTCHATPOPUPCHATDIALOG_H
#define PAINTCHATPOPUPCHATDIALOG_H

#include <gui/chat/PopupChatDialog.h>
#include <QToolButton>
#include "paintchatwindow.h"

class PaintChatPopupChatDialog : public QObject, public PopupChatDialog_WidgetsHolder
{
    Q_OBJECT
public:
    PaintChatPopupChatDialog();
    virtual void init(const std::string &peerId, const QString &title, ChatWidget* chatWidget);
    virtual std::vector<QWidget*> getWidgets();
    virtual void updateStatus(int status){ return;}

private slots:
    void togglePaintChatWindow();

protected:

private:
    QToolButton *paintChatWindowToggleButton;
    PaintChatWindow *paintChatWindow;
};

#endif // PAINTCHATPOPUPCHATDIALOG_H
