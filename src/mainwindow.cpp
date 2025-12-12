#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QLabel>
#include <QGroupBox>
#include <iostream>
#include <QButtonGroup>



void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);
    QHBoxLayout *hLayout = new QHBoxLayout; // horizontal alignment
    QVBoxLayout *vLayout = new QVBoxLayout(); // vertical alignment
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(aspectRatioWidget, 1);
    this->setLayout(hLayout);

    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    // QLabel *controlsLabel = new QLabel();
    // controlsLabel->setText("Controls");
    // controlsLabel->setFont(font);

    QString btnStyle = R"(
        QPushButton {
            background-color: #f0f0f0;
            border: 1px solid #aaa;
            border-radius: 8px;
        }
        QPushButton:checked {
            background-color: #696969;
        }
    )";

    auto makeIconButton = [&](const QString &iconPath) {
        QPushButton *btn = new QPushButton();
        btn->setIcon(QIcon(iconPath));
        btn->setIconSize(QSize(48, 48));
        btn->setFixedSize(64, 64);       // square button
        btn->setCheckable(true);
        btn->setStyleSheet(btnStyle);
        btn->setFlat(true);              // optional style
        return btn;
    };

    treeMode  = makeIconButton(":/svg/tree.svg");
    rockMode  = makeIconButton(":/svg/rock.svg");
    rakeMode  = makeIconButton(":/svg/rake.svg");
    camMode  = makeIconButton(":/svg/cam.svg");
    lightMode  = makeIconButton(":/svg/light.svg");


    QButtonGroup *group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(treeMode);
    group->addButton(rockMode);
    group->addButton(rakeMode);
    group->addButton(camMode);
    group->addButton(lightMode);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setAlignment(Qt::AlignCenter);
    buttonLayout->setSpacing(12);

    buttonLayout->addWidget(treeMode);
    buttonLayout->addWidget(rockMode);
    buttonLayout->addWidget(rakeMode);
    buttonLayout->addWidget(camMode);
    buttonLayout->addWidget(lightMode);


    vLayout->addStretch();
    vLayout->addLayout(buttonLayout);
    vLayout->addStretch();

    connectUIElements();

    camMode->setChecked(true);
    settings.camMode = true;
    realtime->settingsChanged();
}


void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    connectTreeMode();
    connectRockMode();
    connectRakeMode();
    connectCamMode();
    connectLightMode();
}

void MainWindow::connectTreeMode() {
    connect(treeMode, &QPushButton::clicked, this, &MainWindow::onTreeMode);
}
void MainWindow::onTreeMode() {
    treeMode->setChecked(true);
    rockMode->setChecked(false);
    rakeMode->setChecked(false);
    camMode->setChecked(false);

    settings.treeMode = true;
    settings.rockMode = false;
    settings.rakeMode = false;
    settings.camMode = false;
    realtime->settingsChanged();
}

void MainWindow::connectRockMode() {
    connect(rockMode, &QPushButton::clicked, this, &MainWindow::onRockMode);
}
void MainWindow::onRockMode() {
    treeMode->setChecked(false);
    rockMode->setChecked(true);
    rakeMode->setChecked(false);
    camMode->setChecked(false);

    settings.treeMode = false;
    settings.rockMode = true;
    settings.rakeMode = false;
    settings.camMode = false;
    realtime->settingsChanged();
}

void MainWindow::connectRakeMode() {
    connect(rakeMode, &QPushButton::clicked, this, &MainWindow::onRakeMode);
}
void MainWindow::onRakeMode() {
    treeMode->setChecked(false);
    rockMode->setChecked(false);
    rakeMode->setChecked(true);
    camMode->setChecked(false);

    settings.treeMode = false;
    settings.rockMode = false;
    settings.rakeMode = true;
    settings.camMode = false;
    realtime->settingsChanged();
}

void MainWindow::connectCamMode() {
    connect(camMode, &QPushButton::clicked, this, &MainWindow::onCamMode);
}
void MainWindow::onCamMode() {
    treeMode->setChecked(false);
    rockMode->setChecked(false);
    rakeMode->setChecked(false);
    camMode->setChecked(true);

    settings.treeMode = false;
    settings.rockMode = false;
    settings.rakeMode = false;
    settings.camMode = true;
    realtime->settingsChanged();
}

void MainWindow::connectLightMode() {
    connect(lightMode, &QPushButton::clicked, this, &MainWindow::onLightMode);
}
void MainWindow::onLightMode() {
    treeMode->setChecked(false);
    rockMode->setChecked(false);
    rakeMode->setChecked(false);
    camMode->setChecked(false);

    lightMode->setChecked(false);

    settings.treeMode = false;
    settings.rockMode = false;
    settings.rakeMode = false;
    settings.camMode = false;

    settings.lightMode = false;
    realtime->settingsChanged();
}
