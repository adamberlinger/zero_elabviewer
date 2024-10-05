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
#include "RangeControl.h"

static const char* hzLabels[] = {
    "Hz","kHz","kHz","MHz"
};

RangeControl::RangeControl(QString captionFormat, float value){
    this->setLayout(layout = new QGridLayout());
    this->buttonGroup = new QButtonGroup(this);

    range1Button = new QRadioButton("1");
    range2Button = new QRadioButton("100");
    range3Button = new QRadioButton("1k");
    range4Button = new QRadioButton("100 k");

    rangeMultiplier = 1.0f;
    if(value <= 1e3f){
        multiplier = 1.0f;
        range1Button->setChecked(true);
    }
    else if(value <= 1e5f){
        multiplier = 1e3f;
        range2Button->setChecked(true);
        rangeMultiplier = 0.1f;
    }
    else if(value <= 1e6f){
        multiplier = 1e3f;
        range3Button->setChecked(true);
    }
    else {
        multiplier = 1e6f;
        range4Button->setChecked(true);
        rangeMultiplier = 0.1f;
    }
    this->sliderControl = new SliderControl(captionFormat, value / multiplier, 100.0 * rangeMultiplier, 10000.0f * rangeMultiplier, rangeMultiplier);

    layout->addWidget(sliderControl,0,0,1,4);
    layout->addWidget(range1Button,1,0);
    layout->addWidget(range2Button,1,1);
    layout->addWidget(range3Button,1,2);
    layout->addWidget(range4Button,1,3);

    buttonGroup->addButton(range1Button,1);
    buttonGroup->addButton(range2Button,2);
    buttonGroup->addButton(range3Button,3);
    buttonGroup->addButton(range4Button,4);

    QObject::connect(sliderControl, SIGNAL(valueChanged(float)), this, SLOT(inputChanged(float)));
    QObject::connect(sliderControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(inputChangedDelayed(float)));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(rangeChanged(int)));
#else
    QObject::connect(buttonGroup, SIGNAL(idPressed(int)), this, SLOT(rangeChanged(int)));
#endif
}

void RangeControl::rangeChanged(int button){
    float oldMultiplier = rangeMultiplier;
    rangeMultiplier = 1.0f;
    if(button == 1){
        multiplier = 1.0f;
    }
    else if(button == 2){
        multiplier = 1e3f;
        rangeMultiplier = 0.1f;
    }
    else if(button == 3){
        multiplier = 1e3f;
    }
    else if(button == 4){
        multiplier = 1e6f;
        rangeMultiplier = 0.1f;
    }

    float value = sliderControl->getValue();
    sliderControl->setRange(value * (rangeMultiplier / oldMultiplier), 100.0f * rangeMultiplier, 10000.0f * rangeMultiplier, rangeMultiplier);

    if(button >= 1 && button <= 4){
        sliderControl->setUnitLabel(hzLabels[button-1]);
    }
    this->valueChanged(sliderControl->getValue() * multiplier);
    this->valueChangedDelayed(sliderControl->getValue() * multiplier);
}

void RangeControl::inputChanged(float value){
    this->valueChanged(value * multiplier);
}
void RangeControl::inputChangedDelayed(float value){
    this->valueChangedDelayed(value * multiplier);
}

float RangeControl::getValue(){
    return this->sliderControl->getValue() * multiplier;
}

RangeControl::~RangeControl(){}
