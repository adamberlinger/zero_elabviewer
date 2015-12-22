/*
 * Copyright 2016 Adam Berlinger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "RunningIcon.h"

#include <QHBoxLayout>

QPixmap *RunningIcon::icons = NULL;

void RunningIcon::initPixmaps(){
    if(icons == NULL){
        icons = new QPixmap[3];
    }
    if(icons[0].isNull()){
        icons[0].load(":/img/sem_red.png");
        icons[0] = icons[0].scaledToHeight(12,Qt::SmoothTransformation);
    }
    if(icons[1].isNull()){
        icons[1].load(":/img/sem_orange.png");
        icons[1] = icons[1].scaledToHeight(12,Qt::SmoothTransformation);
    }
    if(icons[2].isNull()){
        icons[2].load(":/img/sem_green.png");
        icons[2] = icons[2].scaledToHeight(12,Qt::SmoothTransformation);
    }
}

RunningIcon::RunningIcon(int timeout){
    RunningIcon::initPixmaps();

    timer = new QTimer();
    timer->setSingleShot(true);

    this->timeout = timeout;
    this->setLayout(layout = new QHBoxLayout());

    layout->addWidget(iconLabel = new QLabel());
    layout->addWidget(textLabel = new QLabel("Stopped"));

    iconLabel->setPixmap(icons[0]);
    this->isActive = false;
    this->isRunning = false;

    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timeoutEvent()));
}

void RunningIcon::ping(){
    textLabel->setText("Running");
    this->isActive = true;

    iconLabel->setPixmap(icons[2]);

    /* TODO: reset timer */
    timer->start(timeout);
}

void RunningIcon::stop(){
    this->isRunning = false;
    if(!this->isActive){
        textLabel->setText("Stopped");
        iconLabel->setPixmap(icons[0]);
    }
}

void RunningIcon::start(){
    this->isRunning = true;
    if(!this->isActive){
        textLabel->setText("Waiting");
        iconLabel->setPixmap(icons[1]);
    }
}

void RunningIcon::timeoutEvent(){
    this->isActive = false;
    if(this->isRunning){
        iconLabel->setPixmap(icons[1]);
        textLabel->setText("Waiting");
    }
    else {
        iconLabel->setPixmap(icons[0]);
        textLabel->setText("Stopped");
    }
}

RunningIcon::~RunningIcon(){

}
