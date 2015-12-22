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
#include "PulseCounterWidget.h"
#include <iostream>

static const char* hzLabels[3] = {
    "Hz",
    "kHz",
    "MHz"
};

PulseCounterWidget::PulseCounterWidget(Protocol* protocol, int channel){
    this->protocol = protocol;
    this->protocolChannel = channel;

    this->setLayout(mainLayout = new QVBoxLayout);

    mainLayout->addWidget(frequencyLabel = new QLabel(QString("Frequency: ")));

    mainLayout->addWidget(startButton = new QPushButton("Start"));
    mainLayout->addWidget(stopButton = new QPushButton("Stop"));
    mainLayout->addWidget(runningIcon = new RunningIcon());

    QObject::connect (protocol, SIGNAL(binaryReceived()), this, SLOT(displayData()));
    QObject::connect (protocol, SIGNAL(deviceReconnected()), this, SLOT(configureAll()));

    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startPWM()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopPWM()));
    QObject::connect (startButton, SIGNAL(pressed()), runningIcon, SLOT(start()));
    QObject::connect (stopButton, SIGNAL(pressed()), runningIcon, SLOT(stop()));

    this->setMinimumWidth(300);
}

void PulseCounterWidget::displayData(){
    BinaryTransfer* transfer = protocol->popTransfer(this->protocolChannel);
    if(transfer){
        const uint32_t* rawData = (const uint32_t*)transfer->getData();

        double frequency = ((double)rawData[0]);

        double displayFreq = frequency;
        int range = 0;
        while(displayFreq > 1000 && range < 3){
            displayFreq *= 0.001;
            range++;
        }

        frequencyLabel->setText(QString("Frequency: %1 %2").arg(displayFreq,7,'f',3).arg(hzLabels[range]));

        yieldFrequency(frequency);
        runningIcon->ping();
    }
}

void PulseCounterWidget::configureAll(){
}

void PulseCounterWidget::startPWM(){
    protocol->command('S',this->protocolChannel,1);
}
void PulseCounterWidget::stopPWM(){
    protocol->command('S',this->protocolChannel,0);
}

PulseCounterWidget::~PulseCounterWidget(){}
