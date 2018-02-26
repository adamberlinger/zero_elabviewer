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
#include "VoltmeterWidget.h"
#include <iostream>

static const char* select_voltage[] = {
    "Voltage1",
    "Voltage2",
    "Voltage3",
    "Reference voltage",
    "V2 - V1",
    "V3 - V2",
};

VoltmeterWidget::VoltmeterWidget(Protocol* protocol,int channel, DataConverter* adcConverter){
    this->protocol = protocol;
    this->protocolChannel = channel;
    this->adcConverter = adcConverter;

    this->setLayout(mainLayout = new QVBoxLayout);

    averageSamples = new SliderControl("Number of samples: %1",1.0f, 1.0f,256.0f,1.0f);

    mainLayout->addWidget(voltageLabel = new QLabel("Voltage1: "));
    mainLayout->addWidget(voltageLabel2 = new QLabel("Voltage2: "));
    mainLayout->addWidget(voltageLabel3 = new QLabel("Voltage3: "));
    mainLayout->addWidget(refVoltageLabel = new QLabel("Vdda: "));
    mainLayout->addWidget(diffVoltageLabel1 = new QLabel("V2 - V1: "));
    mainLayout->addWidget(diffVoltageLabel2 = new QLabel("V3 - V2: "));

    mainLayout->addWidget(averageSamples);
    mainLayout->addWidget(startButton = new QPushButton("Start"));
    mainLayout->addWidget(stopButton = new QPushButton("Stop"));
    mainLayout->addWidget(new QLabel("Recording source:"));
    mainLayout->addWidget(recordSelect = new QComboBox());
    mainLayout->addWidget(showRecordButton = new QPushButton("Show recording"));
    mainLayout->addWidget(showDCButton = new QPushButton("Show DC analysis"));

    QObject::connect (protocol, SIGNAL(binaryReceived()), this, SLOT(displayData()));
    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startVoltmeter()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopVoltmeter()));

    QObject::connect (averageSamples, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureNumSamples(float)));

    recordWidget = new MultiRecordWidget("Average voltage - voltmeter","Voltage",true,5.0);
    recordWidget->setWindowTitle("Average Voltage");
    //QObject::connect (this, SIGNAL(yieldVoltage(float,float)), recordWidget, SLOT(recordSimple(float)));
    QObject::connect (showRecordButton, SIGNAL(pressed()), recordWidget, SLOT(show()));

    responseMeasurement = new ResponseMeasurement("DC analysis","Voltage (V)","Voltage (V)","V","V");
    responseMeasurement->setWindowTitle("DC analysis");

    QObject::connect (showDCButton, SIGNAL(pressed()), responseMeasurement, SLOT(show()));
    QObject::connect (this, SIGNAL(yieldVoltage(float,float)), responseMeasurement, SLOT(input(float)));
    QObject::connect (responseMeasurement, SIGNAL(startSignal()), this, SLOT(startVoltmeter()));
    QObject::connect (recordSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRecordingSource(int)));

    for(int i = 0;i < 6;++i){
        recordSelect->addItem(select_voltage[i]);
    }
    recordIndex = 0;
}

void VoltmeterWidget::selectRecordingSource(int index){
    if(index < 6 && index >= 0){
        recordIndex = index;
    }
}

void VoltmeterWidget::setGenerator(GeneratorWidget* generator){
    QObject::connect (responseMeasurement, SIGNAL(output(float)), generator, SLOT(setStaticVoltage(float)));
    QObject::connect (responseMeasurement, SIGNAL(startSignal()), generator, SLOT(startGenerator()));
}

void VoltmeterWidget::configureNumSamples(float value){
    protocol->command('A',this->protocolChannel,(uint16_t)std::floor(value + 0.5));
}

void VoltmeterWidget::displayData(){
    BinaryTransfer* transfer = protocol->popTransfer(this->protocolChannel);
    if(transfer){
        const uint16_t* rawData = (const uint16_t*)transfer->getData();
        int32_t *rawValue = (int32_t*)(rawData + 1);
        /* TODO: check index */

        float voltages[6];

        int channels = (transfer->getSize() - 2) / 4;

        voltages[0] = this->adcConverter->fromDevice(rawValue[0]);
        if(channels > 2)
            voltages[1] = this->adcConverter->fromDevice(rawValue[1]);
        if(channels > 3)
            voltages[2] = this->adcConverter->fromDevice(rawValue[2]);

        voltages[3] = rawValue[channels-1] * 0.001f;

        voltages[0] /= rawData[0]; /* Divide by average samples */
        voltages[1] /= rawData[0];
        voltages[2] /= rawData[0];
        voltages[3] /= rawData[0];

        voltages[4] = voltages[1] - voltages[0];
        voltages[5] = voltages[2] - voltages[1];

        voltageLabel->setText(QString("Voltage1: %1 V").arg(voltages[0],7,'f',3));
        voltageLabel2->setText(QString("Voltage2: %1 V").arg(voltages[1],7,'f',3));
        voltageLabel3->setText(QString("Voltage3: %1 V").arg(voltages[2],7,'f',3));
        refVoltageLabel->setText(QString("Vdda: %1 V").arg(voltages[3],7,'f',3));

        diffVoltageLabel1->setText(QString("V2 - V1: %1V").arg(voltages[4],7,'f',3));
        diffVoltageLabel2->setText(QString("V3 - V2: %1V").arg(voltages[5],7,'f',3));

        recordWidget->recordPrepare(rawData[0] * 0.01f);
        for(int i = 0;i < 4;++i){
            recordWidget->record(voltages[i],i);
        }
        recordWidget->recordSubmit();

        yieldVoltage(voltages[recordIndex], rawData[0] * 0.01f);
    }
}

void VoltmeterWidget::startVoltmeter(){
    protocol->command('S',this->protocolChannel,1);
}
void VoltmeterWidget::stopVoltmeter(){
    protocol->command('S',this->protocolChannel,0);
}

VoltmeterWidget::~VoltmeterWidget(){
    delete recordWidget;
}
