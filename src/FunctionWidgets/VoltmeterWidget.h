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
#ifndef _VOLTMETER_WIDGET_H_
#define _VOLTMETER_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include "qcustomplot.h"
#include "Protocol.h"
#include "RangeControl.h"
#include "RecordWidget.h"
#include "ResponseMeasurement.h"
#include "GeneratorWidget.h"

class VoltmeterWidget : public QWidget {
    Q_OBJECT
protected:
    Protocol* protocol;
    int protocolChannel;
    DataConverter* adcConverter;
    QBoxLayout* mainLayout;
    SliderControl* averageSamples;

    QLabel* voltageLabel;
    QLabel* voltageLabel2;
    QLabel* voltageLabel3;
    QLabel* refVoltageLabel;

    QLabel* diffVoltageLabel1;
    QLabel* diffVoltageLabel2;
    QComboBox* recordSelect;
    int recordIndex;

    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* showRecordButton;
    QPushButton* showDCButton;

    RecordWidget* recordWidget;
    ResponseMeasurement* responseMeasurement;
public slots:
    void startVoltmeter();
    void stopVoltmeter();
    void displayData();
    void configureNumSamples(float value);
    void selectRecordingSource(int index);
signals:
    void yieldVoltage(float value,float time);
public:
    VoltmeterWidget(Protocol* protocol, int channel, DataConverter* adcConverter);
    void setGenerator(GeneratorWidget* generator);
    virtual ~VoltmeterWidget();
};

#endif /* _VOLTMETER_WIDGET_H_ */
