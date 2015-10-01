#ifndef MYQGRAPHICSVIEW_H
#define MYQGRAPHICSVIEW_H

#include <QGraphicsView>

class myQGraphicsView : public QGraphicsView
{
public:
    explicit myQGraphicsView(QWidget *parent = 0) : QGraphicsView(parent) { };
    ~myQGraphicsView() { };

private:
    void resizeEvent(QResizeEvent *event);
};

#endif // MYQGRAPHICSVIEW_H
