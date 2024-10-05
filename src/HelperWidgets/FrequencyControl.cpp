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
#include "FrequencyControl.h"
#include <iostream>

static int rangeNumbers[] = {
    1, 2, 5
};

static const char* unitLabels[] = {
    "Hz","kHz","MHz"
};

FrequencyControl::FrequencyControl(QString caption, float value, bool showRealFrequency){
    int row;
    this->value = value;
    realValue = value;
    slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(7*3);
    slider->setSingleStep(1);
    slider->setPageStep(1);
    slider->setValue(floatToRangeIndex(value));
    this->selectedRange = RANGE_DEFAULT;
    this->buttonGroup = new QButtonGroup(this);

    this->setLayout(layout = new QGridLayout());

    range1Button = new QRadioButton("Coarse");
    range2Button = new QRadioButton("Fine");
    range1Button->setChecked(true);

    buttonGroup->addButton(range1Button,1);
    buttonGroup->addButton(range2Button,2);

    layout->addWidget(captionLabel = new QLabel(caption),0,0);
    layout->addWidget(lineEdit = new QLineEdit(QString::number(value,'f',3)),0,1);
    layout->addWidget(unitLabel = new QLabel("Hz"),0,2);

    row = 1;

    if(showRealFrequency){
        realFrequency = new QLabel(QString("Real value: %1").arg(value,0,'f',3));
        layout->addWidget(realFrequency,row,0,1,3);
        row++;
    }
    else {
        realFrequency = NULL;
    }

    layout->addWidget(slider,row,0,1,3);
    row++;

    layout->addWidget(range1Button,row,0);
    layout->addWidget(range2Button,row,1);
    lineEdit->setAlignment(Qt::AlignRight);

    timer = new QTimer();
    timer->setSingleShot(true);

    this->scale = 1.0f;
    this->changingValue = false;

    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(inputValue(int)));
    QObject::connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(inputValue()));
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(rangeChanged(int)));
#else
    QObject::connect(buttonGroup, SIGNAL(idPressed(int)), this, SLOT(rangeChanged(int)));
#endif
}

void FrequencyControl::setRealValue(float realValue){
    if(realFrequency){
        this->realValue = realValue;
        double x;
        const char* units = DataConverter::getUnitPrefix(realValue, &x);
        realFrequency->setText(QString("Real value: %1 %2").arg(x,0,'f',6).arg(units));
    }
    /*else {
        float error = std::abs((realValue / value) - 1.0f);
        if(error > 0.001){
            int newSliderValue = floatToRangeIndex(realValue);
            slider->setValue(newSliderValue);
        }
    }*/
}

void FrequencyControl::inputValue(int value){
    if(changingValue){
        return;
    }
    changingValue = true;

    if(selectedRange == RANGE_DEFAULT){
        this->value = rangeIndexToFloat(value);
        startRange = rangeIndexToFloat(value-1);
        endRange = rangeIndexToFloat(value+1);
    }
    else {
        this->value = (float)value;
    }
    this->setupTimer();
    if(this->value > 1e6){
        this->lineEdit->setText(QString::number(this->value * 1e-6,'f',3));
        this->unitLabel->setText(unitLabels[2]);
        this->scale = 1e6;
    }
    else if(this->value > 1e3){
        this->lineEdit->setText(QString::number(this->value * 1e-3,'f',3));
        this->unitLabel->setText(unitLabels[1]);
        this->scale = 1e3;
    }
    else {
        this->lineEdit->setText(QString::number(this->value,'f',3));
        this->unitLabel->setText(unitLabels[0]);
        this->scale = 1;
    }
    realValue = value;
    valueChanged(value);

    changingValue = false;
}


float FrequencyControl::rangeIndexToFloat(int input){
    if(input >= 0){
        return rangeNumbers[input % 3] * pow(10,input / 3);
    }
    else {
        return 0.0f;
    }
}

int FrequencyControl::floatToRangeIndex(float input){
    int tens = 0;
    int index = 0;
    while(input > 15.0f){
        input *= 0.1f;
        tens++;
    }
    if(input < 1.5f){
        index = 0;
    }
    else if(input < 3.5) {
        index = 1;
    }
    else if(input < 7.5){
        index = 2;
    }
    else {
        index = 3;
    }
    index += tens * 3;
    return index;
}

void FrequencyControl::rangeChanged(int button){
    if(changingValue){
        return;
    }

    if(button == 1){
        selectedRange = RANGE_DEFAULT;
        int newSliderValue = floatToRangeIndex(this->value);
        slider->setMinimum(0);
        slider->setMaximum(7*3);
        slider->setSingleStep(1);
        slider->setPageStep(1);
        slider->setValue(newSliderValue);
    }
    else {
        selectedRange = RANGE_FINE;
        int newSliderValue = this->value;
        slider->setMinimum(std::floor(startRange + 0.5));
        slider->setMaximum(std::floor(endRange + 0.5));
        slider->setSingleStep(1);
        slider->setPageStep(1);
        slider->setValue(newSliderValue);
    }
}

void FrequencyControl::inputValue(){
    bool status;
    const QString& value = lineEdit->text();
    float newValue = value.toFloat(&status) * this->scale;

    if(std::abs(newValue - this->value) < 0.0001f){
        return;
    }

    float limit_margin = std::min(newValue - 5000000,1 - newValue);

    if(changingValue){
        return;
    }
    changingValue = true;

    if(status && (limit_margin < 0.1f)){
        selectedRange = RANGE_FINE;
        this->value = newValue;
        int range = floatToRangeIndex(newValue);
        startRange = rangeIndexToFloat(range-1);
        endRange = rangeIndexToFloat(range+1);
        slider->setMinimum(std::floor(startRange + 0.5));
        slider->setMaximum(std::floor(endRange + 0.5));
        slider->setSingleStep(1);
        slider->setPageStep(1);
        slider->setValue(newValue);
        range2Button->setChecked(true);

        valueChanged(newValue);

        this->value = newValue;
        realValue = this->value;

        this->setupTimer();
    }
    changingValue = false;
}

void FrequencyControl::timerEvent(){
    valueChangedDelayed(this->value);
}

void FrequencyControl::setupTimer(){
    if(!timer->isActive()){
        timer->start(200);
    }
}

FrequencyControl::~FrequencyControl(){}
