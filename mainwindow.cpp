#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPrintDialog>
#include <QPrinter>

QChar zero('0');
QChar space(' ');

void setPenColor(QGraphicsItem *item, QColor color) {
    QPen pen;

    QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem *>(item);
    if (lineItem) {
        pen = lineItem->pen();
        pen.setColor(color);
        lineItem->setPen(pen);
        return;
    }

    myGraphicsEllipseItem *circleItem = dynamic_cast<myGraphicsEllipseItem *>(item);
    if (circleItem) {
        pen = circleItem->pen();
        pen.setColor(color);
        circleItem->setPen(pen);
        return;
    }
}

void setFontBoldness(QGraphicsSimpleTextItem *item, bool bold) {
    QFont font = item->font();
    font.setBold(bold);
    item->setFont(font);
}

void setBrushColor(QAbstractGraphicsShapeItem *item, QColor color) {
    QBrush brush = item->brush();
    brush.setColor(color);
    item->setBrush(brush);
}

void setBoldnessAndColor(QGraphicsItem *item, bool bold, QColor color) {
    setPenColor(item, color);
    foreach (QGraphicsItem *child, item->childItems()) {
        QGraphicsSimpleTextItem *textChild = dynamic_cast<QGraphicsSimpleTextItem *>(child);
        if (textChild) {
            setFontBoldness(textChild, bold);
            setBrushColor(textChild, color);
        }
    }
}

int lbPctToHW(int pct) {
    if ((pct < 0) || (pct > 100)) {
        return -1;
    }
    if (pct == 0) {
        return 32769;
    }
    return (100 - (double)pct)*32768/99;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ambValue(50000),
    m_maxAmbValue(90000),
    m_maxAmbTickInitialized(false),
    m_lbPct(60)
{
    QString tooltip;

    ui->setupUi(this);
    ui->graphicsView->setScene(&m_scene);

    m_ambBar = new bar(&m_scene, 50, 50, 300, 20, 90000);
    tooltip = "0059 Ambient Sensor Value (read in the dark with fixture at 100%)";
    m_ambBar->initTick(    "100%",      90000,  &m_amb100PctLabel,     &m_amb100PctLine,   &m_amb100PctValue, tooltip);
    tooltip = "005A Ambient Active Level -- 3B -- RW -- Target Sensor Value when in the Active State";
    m_ambBar->initTick(  "Active",      65000,  &m_ambActiveLabel,     &m_ambActiveLine,   &m_ambActiveValue, tooltip);
    tooltip = "005B Ambient Inactive Level -- 3B -- RW -- Target Sensor Value when in the Inactive State";
    m_ambBar->initTick("Inactive",      35000, &m_ambInactiveLabel,  &m_ambInactiveLine, &m_ambInactiveValue, tooltip);
    tooltip = "0059 Ambient Sensor Value (read in the dark with fixture at 1%)";
    m_ambBar->initTick(      "1%",       9000,     &m_amb1PctLabel,      &m_amb1PctLine,     &m_amb1PctValue, tooltip);
    tooltip = "0059 Ambient Sensor Value (read in the dark with fixture at 0%)";
    m_ambBar->initTick(      "0%",       5000,     &m_amb0PctLabel,      &m_amb0PctLine,     &m_amb0PctValue, tooltip);
    tooltip = "0059 Ambient Sensor Value (lowest possible sensor reading of 0)";
    m_ambBar->initTick(    "Dark",          0,     &m_ambDarkLabel,      &m_ambDarkLine,     &m_ambDarkValue, tooltip);

    tooltip = "0059 Ambient Sensor Value -- 3B -- R -- Auto Gained result of the ambient light sensor";
    m_ambBar->initTick( "Current", m_ambValue,  &m_ambCurrentLabel, &m_ambCurrentCircle,  &m_ambCurrentValue, tooltip);
    setBoldnessAndColor(&m_ambCurrentCircle, true, Qt::blue);

    m_ambBar->initBar(&m_ambBarLine);

    qreal y100 = m_amb100PctLine.scenePos().y();
    qreal y1   = m_amb1PctLine.scenePos().y();
    qreal y0   = m_amb0PctLine.scenePos().y();

    m_lbBar = new bar(&m_scene, 300, y100, y1 - y100, 20, -32768, true, y0, 32769);
    tooltip = "Maximum possible light bar brightness";
    m_lbBar->initTick(        "100%", lbPctToHW(    100),   &m_lb100PctLabel,    &m_lb100PctLine,   &m_lb100PctValue, tooltip);
    tooltip = "0008 Light Override Active Level (FFFF is no override) -- 2B -- RW";
    m_lbBar->initTick(  "Active 80%", lbPctToHW(     80),   &m_lbActiveLabel,    &m_lbActiveLine,   &m_lbActiveValue, tooltip);
    tooltip = "0009 Light Override Inactive Level (FFFF is no override) -- 2B -- RW";
    m_lbBar->initTick("Inactive 40%", lbPctToHW(     40), &m_lbInactiveLabel,  &m_lbInactiveLine, &m_lbInactiveValue, tooltip);
    tooltip = "Minimum possible light bar brightness";
    m_lbBar->initTick(  "Min Dim 1%", lbPctToHW(      1),     &m_lb1PctLabel,      &m_lb1PctLine,     &m_lb1PctValue, tooltip);
    tooltip = "Light bars off";
    m_lbBar->initTick(      "Off 0%", lbPctToHW(      0),     &m_lb0PctLabel,      &m_lb0PctLine,     &m_lb0PctValue, tooltip);

    tooltip = "001C Current Light Level -- 4B -- R";
    m_lbBar->initTick("Current XXX%", lbPctToHW(m_lbPct),  &m_lbCurrentLabel, &m_lbCurrentCircle,  &m_lbCurrentValue, tooltip);
    m_lbCurrentLabel.setText(QString("Current %1%").arg(m_lbPct, 3, 10, space));
    setBoldnessAndColor(&m_lbCurrentCircle, true, Qt::blue);

    m_lbBar->initBar(&m_lbBarLine);

    connect(&m_sensorTimer, SIGNAL(timeout()), this, SLOT(sensorTick()));
    m_sensorTimer.setInterval(1000);

    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, 'S', Qt::NoModifier);
    keyPressEvent(k);

    return;
}

MainWindow::~MainWindow()
{
    // Remove all of the items from m_scene. Otherwise, the scene has ownership of the items and
    // calls their destructors from within its destructor, causing a segfault when this object
    // tries to call their destructors for the second time at the end of this destructor.
    foreach (QGraphicsItem *item, m_scene.items()) {
        m_scene.removeItem(item);
    }

    delete ui;
}

void MainWindow::sensorUIUpdate(bool sensorActive)
{
    QColor activeItemColor =   (sensorActive ? Qt::black     : Qt::lightGray);
    QColor inactiveItemColor = (sensorActive ? Qt::lightGray : Qt::black);

    setBoldnessAndColor(  &m_ambActiveLine,  sensorActive,   activeItemColor);
    setBoldnessAndColor(&m_ambInactiveLine, !sensorActive, inactiveItemColor);
    setBoldnessAndColor(   &m_lbActiveLine,  sensorActive,   activeItemColor);
    setBoldnessAndColor( &m_lbInactiveLine, !sensorActive, inactiveItemColor);
}

void MainWindow::keyPressEvent(QKeyEvent *k)
{
    switch (k->key())
    {
    case Qt::Key_S:
        ui->sensorStatus_label->setText("Active");
        m_sensorTimer.start();
        m_sensorCounter = 35;
        sensorUIUpdate(true);
        return;

    case Qt::Key_Q:
    case Qt::Key_A:
        // Adjust m_ambValue upward or downward.
        if (Qt::Key_Q == k->key()) {
            m_ambValue += 1000;
            if (m_ambValue > 120000) {
                m_ambValue = 120000;
            }
        } else {
            m_ambValue -= 1000;
            if (m_ambValue < 0) {
                m_ambValue = 0;
            }
        }

        // Do we have a new maximum value?
        if (m_ambValue > m_maxAmbValue) {
            // Update the max value.
            m_maxAmbValue = m_ambValue;

            // Create the max tick if it didn't already exist.
            if (!m_maxAmbTickInitialized) {
                QString tooltip = "0059 Ambient Sensor Value (maximum value ever read)";
                m_ambBar->initTick("Max", m_maxAmbValue, &m_ambMaxLabel, &m_ambMaxLine, &m_ambMaxValue, tooltip);
                m_maxAmbTickInitialized = true;
            }

            // Update the tick mark.
            m_ambBar->moveTick(m_ambValue, &m_ambMaxLabel, &m_ambMaxLine, &m_ambMaxValue);

            // Redraw the axis.
            m_ambBarLine.setLine(0, 0, 0, m_ambDarkLine.scenePos().y() - m_ambMaxLine.scenePos().y());
            m_ambBarLine.setPos(m_ambMaxLine.scenePos().x(), m_ambMaxLine.scenePos().y());
        }

        // Now that the max label is positioned, move the tick mark.
        m_ambBar->moveTick(m_ambValue, &m_ambCurrentLabel, &m_ambCurrentCircle, &m_ambCurrentValue);
        break;

    case Qt::Key_E:
    case Qt::Key_D:
        // Adjust m_lbPct upward or downward.
        if (Qt::Key_E == k->key()) {
            m_lbPct++;
            if (m_lbPct > 100) {
                m_lbPct = 100;
            }
        }
        if (Qt::Key_D == k->key()) {
            m_lbPct--;
            if (m_lbPct < 0) {
                m_lbPct = 0;
            }
        }

        // Move the tick mark.
        m_lbBar->moveTick(lbPctToHW(m_lbPct), &m_lbCurrentLabel, &m_lbCurrentCircle, &m_lbCurrentValue);

        // Update the label to reflect the new tick mark position.
        m_lbCurrentLabel.setText(QString("Current %1%").arg(m_lbPct, 3, 10, space));
        break;
    }
}

void MainWindow::sensorTick()
{
    // Count down.
    m_sensorCounter--;

    // Clean up if we're done.
    if (m_sensorCounter == 0) {
        ui->sensorStatus_label->setText("Inactive");
        m_sensorTimer.stop();
        sensorUIUpdate(false);
        return;
    }

    // Otherwise, show the countdown if it's below 30.
    if (m_sensorCounter <= 30) {
        ui->sensorStatus_label->setText(QString("Active (%1 sec)").arg(m_sensorCounter));
    }
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer;

    // Show print dialog.
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;

    // Print the scene on the printer.
    QPainter painter;
    painter.begin(&printer);
    ui->graphicsView->scene()->render(&painter);
    painter.end();
}
