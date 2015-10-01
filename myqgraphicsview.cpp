#include "myqgraphicsview.h"

void
myQGraphicsView::resizeEvent(QResizeEvent *event)
{
    fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);

    QGraphicsView::resizeEvent(event);
}
