#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "myqmainwindow.h"

#include <QDebug>
#include <QLineF>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QKeyEvent>
#include <QLocale>
#include <QObject>
#include <QPainter>
#include <QRectF>
#include <QTimer>

extern QChar zero;
extern QChar space;

class myGraphicsEllipseItem : public QGraphicsEllipseItem {
private:
    void paint(QPainter * painter,
          const QStyleOptionGraphicsItem * option,
          QWidget * widget = 0) {
        painter->setRenderHint(QPainter::Antialiasing);
        QGraphicsEllipseItem::paint(painter, option, widget);
    }
};

class bar {
public:
    explicit bar(QGraphicsScene *scene,
                 qreal xOffset,
                 qreal yOffset,
                 qreal height,
                 qreal tickLength,
                 qreal maxValue,
                 bool  hasSpecialTick = false,
                 qreal ySpecial = 0,
                 qreal specialValue = 0) :
        m_scene(scene),
        m_xOffset(xOffset),
        m_yOffset(yOffset),
        m_height(height),
        m_tickLength(tickLength),
        m_maxValue(maxValue),
        m_hasSpecialTick(hasSpecialTick),
        m_ySpecial(ySpecial),
        m_specialValue(specialValue) { }

    void initBar(QGraphicsLineItem *line) {
        line->setLine(0, 0,
                      0, m_height);
        m_scene->addItem(line);
        line->setPos(m_xOffset, m_yOffset);
    }

    void moveTick(qreal                    value,
                  QGraphicsSimpleTextItem *leftLabel,
                  QGraphicsItem           *tickItem,
                  QGraphicsSimpleTextItem *rightLabel) {
        qreal yCoord;
        // Special case for the special value which doesn't lie on the bar.
        if ((m_hasSpecialTick) && (value == m_specialValue)) {
            yCoord = m_ySpecial;
        } else if (m_maxValue > 0) {
            // Bar increases upwards (opposite of x coordinates)
            yCoord = m_yOffset + m_height*(1 - value/m_maxValue);
        } else {
            // Bar increases downwards (same as x coordinates)
            yCoord = m_yOffset + m_height*(value/(-m_maxValue));
        }

        tickItem->setPos(m_xOffset, yCoord);
        rightLabel->setText(QString("0x%1 (%2)").
                            arg(QString("%1").arg((ulong)value, 6, 16, zero).toUpper()).
                            arg(QLocale::c().toString((qulonglong)value)));

        // Put the labels back in their natural relative positions...
        leftLabel->setPos(-leftLabel->boundingRect().width() - m_tickLength/2 - 4,
                          -leftLabel->boundingRect().height()/2);
        rightLabel->setPos(m_tickLength/2 + 4,
                           -rightLabel->boundingRect().height()/2);
        // ...then resolve any collisions.
        repositionLabelsIfNecessary(leftLabel, rightLabel);
    }

    void initTick(QString                  text,
                  qreal                    value,
                  QGraphicsSimpleTextItem *leftLabel,
                  QGraphicsItem           *tickItem,
                  QGraphicsSimpleTextItem *rightLabel,
                  QString                  tooltip) {
        QGraphicsLineItem *line = dynamic_cast<QGraphicsLineItem *>(tickItem);
        myGraphicsEllipseItem *circle = dynamic_cast<myGraphicsEllipseItem *>(tickItem);
        if (!line && !circle) {
            return;
        }

        m_scene->addItem(tickItem);
        moveTick(value, leftLabel, tickItem, rightLabel);
        tickItem->setToolTip(tooltip);
        if (leftLabel) {
            leftLabel->setParentItem(tickItem);
            leftLabel->setText(text);
            leftLabel->setToolTip(tooltip);
            leftLabel->setPos(-leftLabel->boundingRect().width() - m_tickLength/2 - 4,
                              -leftLabel->boundingRect().height()/2);
        }
        if (line) {
            line->setLine(-m_tickLength/2, 0,
                          +m_tickLength/2, 0);
        } else {
            circle->setRect(-m_tickLength/4, -m_tickLength/4,
                            m_tickLength/2, m_tickLength/2);
        }
        if (rightLabel) {
            rightLabel->setParentItem(tickItem);
            rightLabel->setToolTip(tooltip);
            rightLabel->setPos(m_tickLength/2 + 4,
                               -rightLabel->boundingRect().height()/2);
        }

        repositionLabelsIfNecessary(leftLabel, rightLabel);
    }

private:
    QGraphicsScene *m_scene;
    qreal m_xOffset;
    qreal m_yOffset;
    qreal m_height;
    qreal m_tickLength;
    qreal m_maxValue;
    bool  m_hasSpecialTick;
    qreal m_ySpecial;
    qreal m_specialValue;

    void repositionLabelsIfNecessary(QGraphicsSimpleTextItem *leftLabel,
                                     QGraphicsSimpleTextItem *rightLabel)
    {
        QGraphicsSimpleTextItem *itemWithPotentialCollision;
        QList <QGraphicsItem *> collidingItems;
        int yShift;

        itemWithPotentialCollision = leftLabel;
        collidingItems = itemWithPotentialCollision->collidingItems();
        if (collidingItems.isEmpty()) {
            itemWithPotentialCollision = rightLabel;
            collidingItems = itemWithPotentialCollision->collidingItems();
            if (collidingItems.isEmpty()) {
                return;
            }
        }

//        // Found a collision.
//        QPointF  leftLabelOriginalPos =  leftLabel->pos();
//        QPointF rightLabelOriginalPos = rightLabel->pos();

//        // Measure distance to shift up to avoid collision.
//        int upShift = 0;
//        forever {
//            upShift++;
//             leftLabel->setPos( leftLabel->x(),  leftLabel->y() - 1);
//            rightLabel->setPos(rightLabel->x(), rightLabel->y() - 1);

//            itemWithPotentialCollision = leftLabel;
//            collidingItems = itemWithPotentialCollision->collidingItems();
//            if (collidingItems.isEmpty()) {
//                itemWithPotentialCollision = rightLabel;
//                collidingItems = itemWithPotentialCollision->collidingItems();
//                if (collidingItems.isEmpty()) {
//                    break;
//                }
//            }
//        }

//        // Restore item positions.
//         leftLabel->setPos( leftLabelOriginalPos);
//        rightLabel->setPos(rightLabelOriginalPos);

//        // Measure distance to shift up to avoid collision.
//        int downShift = 0;
//        forever {
//            downShift++;
//             leftLabel->setPos( leftLabel->x(),  leftLabel->y() + 1);
//            rightLabel->setPos(rightLabel->x(), rightLabel->y() + 1);

//            itemWithPotentialCollision = leftLabel;
//            collidingItems = itemWithPotentialCollision->collidingItems();
//            if (collidingItems.isEmpty()) {
//                itemWithPotentialCollision = rightLabel;
//                collidingItems = itemWithPotentialCollision->collidingItems();
//                if (collidingItems.isEmpty()) {
//                    break;
//                }
//            }
//        }

//        // Pick smallest shift.
//        int shift = 0;
//        if (upShift <= downShift) {
//            // Shift up.
//            shift = -upShift;
//        } else {
//            // Shift down.
//            shift = downShift;
//        }

//        // Apply vertical shift.
//         leftLabel->setPos( leftLabelOriginalPos + QPointF(0, shift));
//        rightLabel->setPos(rightLabelOriginalPos + QPointF(0, shift));

        // Found a collision.
        // Do a very simple test to guess the best direction to shift to undo it.
        if ((itemWithPotentialCollision->sceneBoundingRect().center().y()) <
            (      collidingItems.at(0)->sceneBoundingRect().center().y())) {
            // Midline of our label is higher than that of the item it's colliding with.
            // Therefore shift our labels upward until they don't collide with anything.
            yShift = -1;
        } else {
            // Otherwise, shift our labels downward until they don't collide with anything.
            yShift = 1;
        }

        forever {
             leftLabel->setPos( leftLabel->x(),  leftLabel->y() + yShift);
            rightLabel->setPos(rightLabel->x(), rightLabel->y() + yShift);

            itemWithPotentialCollision = leftLabel;
            collidingItems = itemWithPotentialCollision->collidingItems();
            if (collidingItems.isEmpty()) {
                itemWithPotentialCollision = rightLabel;
                collidingItems = itemWithPotentialCollision->collidingItems();
                if (collidingItems.isEmpty()) {
                    return;
                }
            }
        }
    }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QGraphicsScene m_scene;

    bar *m_ambBar;

    QGraphicsLineItem m_ambBarLine;

    QGraphicsSimpleTextItem m_ambMaxLabel;
    QGraphicsSimpleTextItem m_amb100PctLabel;
    QGraphicsSimpleTextItem m_ambActiveLabel;
    QGraphicsSimpleTextItem m_ambCurrentLabel;
    QGraphicsSimpleTextItem m_ambInactiveLabel;
    QGraphicsSimpleTextItem m_amb1PctLabel;
    QGraphicsSimpleTextItem m_amb0PctLabel;
    QGraphicsSimpleTextItem m_ambDarkLabel;

    QGraphicsLineItem     m_ambMaxLine;
    QGraphicsLineItem     m_amb100PctLine;
    QGraphicsLineItem     m_ambActiveLine;
    myGraphicsEllipseItem m_ambCurrentCircle;
    QGraphicsLineItem     m_ambInactiveLine;
    QGraphicsLineItem     m_amb1PctLine;
    QGraphicsLineItem     m_amb0PctLine;
    QGraphicsLineItem     m_ambDarkLine;

    QGraphicsSimpleTextItem m_ambMaxValue;
    QGraphicsSimpleTextItem m_amb100PctValue;
    QGraphicsSimpleTextItem m_ambActiveValue;
    QGraphicsSimpleTextItem m_ambCurrentValue;
    QGraphicsSimpleTextItem m_ambInactiveValue;
    QGraphicsSimpleTextItem m_amb1PctValue;
    QGraphicsSimpleTextItem m_amb0PctValue;
    QGraphicsSimpleTextItem m_ambDarkValue;

    bar *m_lbBar;

    QGraphicsLineItem m_lbBarLine;

    QGraphicsSimpleTextItem m_lb100PctLabel;
    QGraphicsSimpleTextItem m_lbActiveLabel;
    QGraphicsSimpleTextItem m_lbCurrentLabel;
    QGraphicsSimpleTextItem m_lbInactiveLabel;
    QGraphicsSimpleTextItem m_lb1PctLabel;
    QGraphicsSimpleTextItem m_lb0PctLabel;

    QGraphicsLineItem     m_lb100PctLine;
    QGraphicsLineItem     m_lbActiveLine;
    myGraphicsEllipseItem m_lbCurrentCircle;
    QGraphicsLineItem     m_lbInactiveLine;
    QGraphicsLineItem     m_lb1PctLine;
    QGraphicsLineItem     m_lb0PctLine;

    QGraphicsSimpleTextItem m_lb100PctValue;
    QGraphicsSimpleTextItem m_lbActiveValue;
    QGraphicsSimpleTextItem m_lbCurrentValue;
    QGraphicsSimpleTextItem m_lbInactiveValue;
    QGraphicsSimpleTextItem m_lb1PctValue;
    QGraphicsSimpleTextItem m_lb0PctValue;

    int m_ambValue;
    int m_maxAmbValue;
    bool m_maxAmbTickInitialized;
    int m_lbPct;
    QTimer m_sensorTimer;
    int m_sensorCounter;

    void sensorUIUpdate(bool sensorActive);
    void keyPressEvent(QKeyEvent *k);

private slots:
    void sensorTick();
    void on_actionPrint_triggered();
};

#endif // MAINWINDOW_H
