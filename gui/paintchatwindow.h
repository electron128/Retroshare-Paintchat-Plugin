#ifndef PAINTCHATWINDOW_H
#define PAINTCHATWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class PaintChatWindow;
}

class PaintChatWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit PaintChatWindow(QWidget *parent = 0);
    ~PaintChatWindow();

    void setPeerId(std::string peerId);
    
private slots:
    void on_haveUpdate();
    void on_timer();

    void on_pushButtonPrimaryColor_clicked();
    
    void on_pushButtonPen_clicked();

    void on_pushButtonWhite_clicked();

    void on_pushButton1px_clicked();

    void on_pushButton4px_clicked();

    void on_pushButton8px_clicked();

    void on_pushButtonClear_clicked();

    void on_pushButtonCopy_clicked();

private:
    void updateImage();
    void resetPenButtons();
    void colorChanged();
    
    QColor currentColor;
    Ui::PaintChatWindow *ui;
    std::string peerId;

    QTimer *timer;
};

#endif // PAINTCHATWINDOW_H
