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
#include <cmath>

#define VOLTAGE_LIST_SIZE   (7)

static const char* select_voltage[VOLTAGE_LIST_SIZE] = {
    "Voltage1",
    "Voltage2",
    "Voltage3",
    "Reference voltage",
    "V2 - V1",
    "V3 - V2",
    "Input voltage (DAC)"
};

/* Function to filter voltage values to reasonable bounds.
    From time to time strange/error values are computed/measured,
    which can mess up the record graph.
*/
static float volt_filter(float input){
    if(std::isnan(input)) return 0.0f;
    if(input > 10.0f) return 10.0f;
    if(input < -10.0f) return -10.0f;
    return input;
}

VoltmeterWidget::VoltmeterWidget(Protocol* protocol,int channel, DataConverter* adcConverter){
    this->protocol = protocol;
    this->protocolChannel = channel;
    this->adcConverter = adcConverter;

    this->setLayout(mainLayout = new QVBoxLayout);

    averageSamples = new SliderControl("Number of samples: %1",1.0f, 1.0f,256.0f,1.0f);
    responseMeasurement = new ResponseMeasurement("DC analysis","Voltage input (V)","Voltage output (V)","V","V");
    responseMeasurement->setWindowTitle("DC analysis");


    mainLayout->addWidget(voltageLabel = new QLabel("Voltage1: "));
    mainLayout->addWidget(voltageLabel2 = new QLabel("Voltage2: "));
    mainLayout->addWidget(voltageLabel3 = new QLabel("Voltage3: "));
    mainLayout->addWidget(refVoltageLabel = new QLabel("Vdda: "));
    mainLayout->addWidget(diffVoltageLabel1 = new QLabel("V2 - V1: "));
    mainLayout->addWidget(diffVoltageLabel2 = new QLabel("V3 - V2: "));

    mainLayout->addWidget(averageSamples);
    mainLayout->addWidget(startButton = new QPushButton("Start"));
    mainLayout->addWidget(stopButton = new QPushButton("Stop"));
    mainLayout->addWidget(showRecordButton = new QPushButton("Show recording"));
    mainLayout->addWidget(showDCButton = new QPushButton("Show DC analysis"));

    responseMeasurement->controlLayout->addWidget(new QLabel("DC analysis source:"), responseMeasurement->controlRows++, 0, 1, 2);
    responseMeasurement->controlLayout->addWidget(new QLabel("x axis:"), responseMeasurement->controlRows, 0, 1, 1);
    responseMeasurement->controlLayout->addWidget(recordXSelect = new QComboBox(), responseMeasurement->controlRows++, 1, 1, 1);
    responseMeasurement->controlLayout->addWidget(new QLabel("Y axis:"), responseMeasurement->controlRows, 0, 1, 1);
    responseMeasurement->controlLayout->addWidget(recordYSelect = new QComboBox(), responseMeasurement->controlRows++, 1, 1, 1);
    responseMeasurement->controlLayout->setRowStretch(responseMeasurement->controlRows, 1);

    QObject::connect (protocol, SIGNAL(binaryReceived()), this, SLOT(displayData()));
    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startVoltmeter()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopVoltmeter()));

    QObject::connect (averageSamples, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureNumSamples(float)));

    recordWidget = new MultiRecordWidget("Average voltage - voltmeter","Voltage",true,-1.0,4);
    recordWidget->setWindowTitle("Average Voltage");
    QObject::connect (showRecordButton, SIGNAL(pressed()), recordWidget, SLOT(show()));

    QObject::connect (showDCButton, SIGNAL(pressed()), responseMeasurement, SLOT(show()));
    QObject::connect (this, SIGNAL(yieldVoltage2(float,float)), responseMeasurement, SLOT(inputX(float)));
    QObject::connect (this, SIGNAL(yieldVoltage(float,float)), responseMeasurement, SLOT(inputY(float)));
    QObject::connect (responseMeasurement, SIGNAL(startSignal()), this, SLOT(startVoltmeter()));
    QObject::connect (recordXSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRecordingXSource(int)));
    QObject::connect (recordYSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRecordingYSource(int)));

    for(int i = 0;i < VOLTAGE_LIST_SIZE;++i){
        recordXSelect->addItem(select_voltage[i]);
        recordYSelect->addItem(select_voltage[i]);
    }
    recordXSelect->setCurrentIndex(VOLTAGE_LIST_SIZE-1);
    recordXIndex = VOLTAGE_LIST_SIZE-1;
    recordYIndex = 0;
}

void VoltmeterWidget::selectRecordingXSource(int index){
    if(index < VOLTAGE_LIST_SIZE && index >= 0){
        recordXIndex = index;
    }
}

void VoltmeterWidget::selectRecordingYSource(int index){
    if(index < VOLTAGE_LIST_SIZE && index >= 0){
        recordYIndex = index;
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

        float voltages[VOLTAGE_LIST_SIZE-1];

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

        voltages[0] = volt_filter(voltages[0]);
        voltages[1] = volt_filter(voltages[1]);
        voltages[2] = volt_filter(voltages[2]);
        voltages[3] = volt_filter(voltages[3]);

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

        if(recordXIndex < (VOLTAGE_LIST_SIZE-1)){
            yieldVoltage2(voltages[recordXIndex], rawData[0] * 0.01f);
        }
        else {
            responseMeasurement->inputAsOutput(false);
        }
        if(recordYIndex < (VOLTAGE_LIST_SIZE-1)){
            yieldVoltage(voltages[recordYIndex], rawData[0] * 0.01f);
        }
        else {
            responseMeasurement->inputAsOutput(true);
        }
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
