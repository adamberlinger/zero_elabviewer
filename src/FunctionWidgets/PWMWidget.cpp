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
#include "PWMWidget.h"
#include <iostream>

PWMWidget::PWMWidget(Protocol* protocol, int channel){
    this->protocol = protocol;
    this->protocolChannel = channel;

    this->setLayout(mainLayout = new QVBoxLayout);
    frequencyControl = new FrequencyControl("Frequency: ",100.0f, true);
    dutyCycleControl = new SliderControl("Duty cycle: %1 %",50.0f, 0.0f,100.0f,0.1f);

    mainLayout->addWidget(frequencyControl);
    mainLayout->addWidget(dutyCycleControl);

    mainLayout->addWidget(startButton = new QPushButton("Start"));
    mainLayout->addWidget(stopButton = new QPushButton("Stop"));

    QObject::connect (protocol, SIGNAL(deviceReconnected()), this, SLOT(configureAll()));
    QObject::connect (protocol, SIGNAL(commandReceived()), this, SLOT(commandReceived()));

    QObject::connect (frequencyControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureFrequency(float)));
    QObject::connect (dutyCycleControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureDutyCycle(float)));
    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startPWM()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopPWM()));
}

void PWMWidget::commandReceived(){
    const Command* command = protocol->getLastCommand();
    if(command->channel == this->protocolChannel){
        if(command->command_id == 'F'){
            timerFrequency = command->value;
        }
        else if(command->command_id == 'D') {
            if(timerFrequency > 0){
                double realFrequency = ((double)timerFrequency) / command->value;
                frequencyControl->setRealValue(realFrequency);
            }
        }
    }
}

void PWMWidget::configureAll(){
    configureFrequency(frequencyControl->getValue());
    configureDutyCycle(dutyCycleControl->getValue());
}

void PWMWidget::configureFrequency(float value){
    uint16_t device_value = DataConverter::convertSampleRate(value);
    protocol->command('F',this->protocolChannel,device_value);
}

void PWMWidget::configureDutyCycle(float value){
    uint16_t device_value = std::floor(value * 10.0f + 0.5f);
    protocol->command('D',this->protocolChannel,device_value);
}
void PWMWidget::startPWM(){
    protocol->command('S',this->protocolChannel,1);
}
void PWMWidget::stopPWM(){
    protocol->command('S',this->protocolChannel,0);
}

PWMWidget::~PWMWidget(){}
