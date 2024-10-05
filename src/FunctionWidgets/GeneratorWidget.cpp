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
#include "GeneratorWidget.h"
#include <iostream>

GeneratorWidget::GeneratorWidget(Protocol* protocol, int channel){
    this->protocol = protocol;
    this->protocolChannel = channel;

    this->setLayout(mainLayout = new QVBoxLayout);
    frequencyControl = new FrequencyControl("Frequency:",100.0f, true);
    scaleControl = new SliderControl("Amplitude: %1 %",100.0f,0.0f,100.0f,0.1f);
    offsetControl = new SliderControl("Offset: %1 %",0.0f,0.0f,100.0f,0.1f);
    voltageControl = new SliderControl("Voltage: %1 V",0.0f,0.0f,3.3f,0.001f);

    mainLayout->addWidget(frequencyControl);
    mainLayout->addWidget(scaleControl);
    mainLayout->addWidget(offsetControl);
    mainLayout->addWidget(voltageControl);
    mainLayout->addWidget(voltageOutputEnable = new QCheckBox("Force fixed voltage"));
    mainLayout->addWidget(shapeSineButton = new QRadioButton("Sine"));
    mainLayout->addWidget(shapeTriangleButton = new QRadioButton("Triangle"));
    mainLayout->addWidget(shapeSawButton = new QRadioButton("Saw"));
    mainLayout->addWidget(shapeSquareButton = new QRadioButton("Square"));
    mainLayout->addWidget(shapeNoiseButton = new QRadioButton("Noise"));
    mainLayout->addWidget(startButton = new QPushButton("Start"));
    mainLayout->addWidget(stopButton = new QPushButton("Stop"));

    buttonGroup = new QButtonGroup();
    buttonGroup->addButton(shapeSineButton,1);
    buttonGroup->addButton(shapeTriangleButton,2);
    buttonGroup->addButton(shapeSquareButton,3);
    buttonGroup->addButton(shapeNoiseButton,4);
    buttonGroup->addButton(shapeSawButton,5);

    shapeSineButton->setChecked(true);

    QObject::connect (protocol, SIGNAL(deviceReconnected()), this, SLOT(configureAll()));
    QObject::connect (protocol, SIGNAL(commandReceived()), this, SLOT(commandReceived()));

    QObject::connect (frequencyControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureFrequency(float)));
    QObject::connect (scaleControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureScale(float)));
    QObject::connect (offsetControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureOffset(float)));
    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startGenerator()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopGenerator()));

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect(buttonGroup, SIGNAL(buttonPressed(int)), this, SLOT(configureShape(int)));
#else
    QObject::connect(buttonGroup, SIGNAL(idPressed(int)), this, SLOT(configureShape(int)));
#endif
    QObject::connect(voltageControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureVoltage()));
    QObject::connect(voltageOutputEnable, SIGNAL(stateChanged(int)), this, SLOT(configureVoltage()));
}

void GeneratorWidget::commandReceived(){
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

void GeneratorWidget::configureAll(){
    configureFrequency(frequencyControl->getValue());
    configureScale(scaleControl->getValue());
    configureOffset(offsetControl->getValue());
}

void GeneratorWidget::configureFrequency(float value){
    /*uint16_t device_value = DataConverter::convertSampleRate(value);
    protocol->command('F',this->protocolChannel,device_value);*/
    /* TODO: load sample frequency */
    uint32_t addend = (uint32_t)std::floor(((4294967296.0f * value) / 1e6) + 0.5);
    protocol->command('R',this->protocolChannel,addend);
}

void GeneratorWidget::configureShape(int button_id){
    protocol->command('P',this->protocolChannel,(button_id-1));
}

void GeneratorWidget::configureScale(float value){
    uint16_t device_value = value * 0.01f * 0xFFFF;
    protocol->command('A',this->protocolChannel,device_value);
}

void GeneratorWidget::configureOffset(float value){
    uint16_t device_value = value * 0.01f * 0xFFFF;
    protocol->command('O',this->protocolChannel,device_value);
}

void GeneratorWidget::startGenerator(){
    protocol->command('S',this->protocolChannel,1);
}
void GeneratorWidget::stopGenerator(){
    protocol->command('S',this->protocolChannel,0);
}

void GeneratorWidget::configureVoltage(){
    if(voltageOutputEnable->isChecked()){
        double value = (voltageControl->getValue() / 3.3f) * 256 * 256;
        if(value > (256.0f * 256)) value = 256.0*256.0-1;
        uint16_t device_value = (uint16_t)std::floor(value);
        protocol->command('V',this->protocolChannel,device_value);
    }
    else {
        protocol->command('G',this->protocolChannel,0);
    }
}

void GeneratorWidget::setStaticVoltage(float value){
    this->voltageControl->setValue(value);
    this->voltageOutputEnable->setChecked(true);
    configureVoltage();
}

GeneratorWidget::~GeneratorWidget(){}
