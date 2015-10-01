#ifndef MYQMAINWINDOW_H
#define MYQMAINWINDOW_H

#include <QMainWindow>

class myQMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit myQMainWindow(QWidget *parent = 0) : QMainWindow(parent) { };
    ~myQMainWindow() { };

signals:

public slots:
};

#endif // MYQMAINWINDOW_H
