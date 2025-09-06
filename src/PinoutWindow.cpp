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
#include "PinoutWindow.h"

PinoutWindow::PinoutWindow(){
    this->setLayout(this->layout = new QGridLayout());
    this->layout->addWidget(this->title = new QLabel("No pinout loaded"),0,0,1,3,Qt::AlignHCenter);
    this->pinLabels = NULL;
    this->pinCount = 0;
}

void PinoutWindow::displayPinout(const Pinout& pinout){
    this->clearPinout();

    this->pinCount = pinout.getPackageCount();
    this->pinLabels = new QLabel*[(this->pinCount*3)/2];
    int pinoutSize = pinout.getPinCount();
    int li = 0, ri = pinoutSize-1;
    for(int i = 0;i < this->pinCount / 2;++i){
        bool lv = (pinout.getPinPackageNum(li) == (i+1));
        bool rv = (pinout.getPinPackageNum(ri) == (this->pinCount-i));

        if(lv){
          this->pinLabels[i*3] = new QLabel(pinout.getPinLabel("%1(%4) / P%2%3",li));
          li++;
        }
        else {
          this->pinLabels[i*3] = new QLabel("");
        }
        if(rv){
          this->pinLabels[i*3+1] = new QLabel(pinout.getPinLabel("P%2%3 / %1(%4)",ri));
          ri++;
        }
        else {
          this->pinLabels[i*3+1] = new QLabel("");
        }
        this->pinLabels[i*3+2] = new QLabel(QString("|%1 %2|").arg(i+1,2).arg(this->pinCount-i));
        this->layout->addWidget(this->pinLabels[i*3], i+1, 0, Qt::AlignRight);
        this->layout->addWidget(this->pinLabels[i*3+2], i+1, 1, Qt::AlignHCenter);
        this->layout->addWidget(this->pinLabels[i*3+1], i+1, 2, Qt::AlignLeft);
    }
    this->layout->addItem(spacer = new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding),(this->pinCount/2)+1,0,1,3);
    this->layout->update();
}

void PinoutWindow::clearPinout(){
    if(this->pinCount){
        for(int i = 0;i < (this->pinCount*3)/2;++i){
            this->layout->removeWidget(this->pinLabels[i]);
            delete this->pinLabels[i];
        }
        this->layout->removeItem(spacer);
        delete spacer;
        delete[] this->pinLabels;
        this->pinLabels = NULL;
        this->pinCount = 0;
    }
}

PinoutWindow::~PinoutWindow(){

}
