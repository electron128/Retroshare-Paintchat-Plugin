#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QImage>
#include <stdint.h>

class PaintWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PaintWidget(QWidget *parent = 0);
    void setImage(const QImage&);
    QImage getImage();

    void fillImage(QColor color);

    QColor color;
    uint8_t penWidth;
    
signals:
    void haveUpdate();
    
public slots:

protected:
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void paintEvent(QPaintEvent *);

private:
    QImage image;
    
};

#endif // PAINTWIDGET_H
