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
#include "SliderControl.h"
#include <iostream>

SliderControl::SliderControl(QString captionFormat, float value,
    float min, float max,float step):QWidget(){

    this->step = step;
    this->min = min;
    this->max = max;
    this->value = value;
    this->numDigits = std::ceil(std::log10(1 / this->step));
    this->setLayout(layout = new QGridLayout());

    QStringList labelList = captionFormat.split("%1");

    captionLabel = new QLabel(labelList.at(0));
    unitLabel = new QLabel(labelList.at(1));
    lineEdit = new QLineEdit(QString::number(value,'f',numDigits));
    layout->addWidget(captionLabel,0,0);
    layout->addWidget(lineEdit,0,1);
    layout->addWidget(unitLabel,0,2);
    layout->addWidget(slider = new QSlider(Qt::Horizontal),1,0,1,3);

    int imax = std::floor(max / step + 0.5f);
    int imin = std::floor(min / step + 0.5f);
    slider->setMaximum(imax);
    slider->setMinimum(imin);
    slider->setSingleStep(1);
    slider->setPageStep(1);
    slider->setValue(std::floor(value / step + 0.5f));

    lineEdit->setAlignment(Qt::AlignRight);

    timer = new QTimer();
    timer->setSingleShot(true);

    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(inputValue(int)));
    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(inputValue()));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
}

void SliderControl::setRange(float value, float min,float max,float step){
    slider->setMaximum(std::floor(max / step + 0.5f));
    slider->setMinimum(std::floor(min / step + 0.5f));
    slider->setValue(std::floor(value / step + 0.5f));

    this->step = step;
    this->max = max;
    this->min = min;
    this->value = value;
    this->numDigits = std::ceil(std::log10(1 / this->step));
    this->lineEdit->setText(QString::number(this->value,'f',numDigits));
}

void SliderControl::setValue(float value){
    slider->setValue(std::floor(value / step + 0.5f));
    this->value = value;
    this->lineEdit->setText(QString::number(this->value,'f',numDigits));
}

void SliderControl::setUnitLabel(QString value){
    unitLabel->setText(value);
}

float SliderControl::getValue(){
    return this->value;
}

void SliderControl::inputValue(int value){
    this->value = value * this->step;
    this->setupTimer();
    this->lineEdit->setText(QString::number(this->value,'f',numDigits));
    valueChanged(value);
}

void SliderControl::inputValue(){
    bool status;
    QString value = this->lineEdit->text();
    float newValue = value.toFloat(&status);

    float limit_margin = std::min(newValue - max,min - newValue);

    if(status && (limit_margin < (0.1f * this->step))){
        this->value = newValue;
        valueChanged(this->value);
        slider->setValue(std::floor(this->value / this->step + 0.5f));
        this->setupTimer();
    }
}

void SliderControl::timerEvent(){
    valueChangedDelayed(this->value);
}

void SliderControl::setupTimer(){
    if(!timer->isActive()){
        timer->start(200);
    }
}

SliderControl::~SliderControl(){}
