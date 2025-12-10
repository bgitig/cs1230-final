#pragma once

#include <QMainWindow>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();

    void connectTreeMode();
    void connectRockMode();
    void connectRakeMode();
    void connectCamMode();



    Realtime *realtime;
    AspectRatioWidget *aspectRatioWidget;

    QPushButton *treeMode;
    QPushButton *rockMode;
    QPushButton *rakeMode;
    QPushButton *camMode;



private slots:

    void onTreeMode();
    void onRockMode();
    void onRakeMode();
    void onCamMode();

};
