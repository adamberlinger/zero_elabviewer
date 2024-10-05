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
#include "ChannelControl.h"

ChannelControl::ChannelControl(int numChannels,uint32_t channelMask){
    this->numChannels = numChannels;

    this->setLayout(layout = new QGridLayout());
    this->channelEnable = new QCheckBox*[numChannels];
    this->channelTrigger = new QRadioButton*[numChannels];
    this->triggerButtonGroup = new QButtonGroup(this);

    for(int i = 0; i < numChannels;++i){
        layout->addWidget(channelEnable[i] = new QCheckBox(QString("Channel %1").arg(i+1)),i,0);
        layout->addWidget(channelTrigger[i] = new QRadioButton("Trigger"),i,1);
        triggerButtonGroup->addButton(channelTrigger[i],i);
        if(channelMask & (0x1 << i)){
            channelEnable[i]->setChecked(true);
        }
        QObject::connect(channelEnable[i], SIGNAL(clicked()), this, SLOT(checkBoxEvent()));
    }
    channelTrigger[0]->setChecked(true);
    layout->setColumnStretch(0,5);
    layout->setColumnStretch(1,5);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(triggerButtonGroup, SIGNAL(buttonPressed(int)), this, SLOT(radioButtonEvent(int)));
#else
    QObject::connect(triggerButtonGroup, SIGNAL(idPressed(int)), this, SLOT(radioButtonEvent(int)));
#endif
}

uint32_t ChannelControl::getActiveChannelMask(){
    uint32_t channelMask = 0;
    for(int i = 0; i < numChannels;++i){
        if(channelEnable[i]->isChecked()){
            channelMask |= (0x1 << i);
        }
    }
    return channelMask;
}

void ChannelControl::checkBoxEvent(){
    changedActiveChannels(getActiveChannelMask());
}

void ChannelControl::radioButtonEvent(int channel){
    changedTriggerChannel(channel);
}

ChannelControl::~ChannelControl(){

}
