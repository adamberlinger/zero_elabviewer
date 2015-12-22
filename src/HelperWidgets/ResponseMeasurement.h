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
#ifndef _RESPONSE_MEASUREMENT_H_
#define _RESPONSE_MEASUREMENT_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include "ExtendedPlot.h"
#include "SliderControl.h"

class ResponseMeasurement : public QWidget {
    Q_OBJECT
protected:
    ExtendedPlot* plot;
    QBoxLayout* mainLayout;
    QBoxLayout* controlLayout;
    QWidget* sidePanel;

    QPushButton* startButton;
    SliderControl* delayControl;
    SliderControl* stepsControl;
    SliderControl* startValueControl;
    SliderControl* stopValueControl;
    QTimer* timer;

    unsigned numSteps;
    unsigned index;
    float currentOutput;
    float outputStep;

    QVector<double> *xValues;
    QVector<double> *yValues;

    volatile float readedValue;
private slots:
    void startDC();
    void step();
public slots:
    void input(float value);
signals:
    void startSignal();
    void forceInput();
    void output(float value);
public:
    ResponseMeasurement(QString caption, QString xAxisLabel, QString yAxisLabel,
        QString xUnits,QString yUnits);
    virtual ~ResponseMeasurement();
};

#endif
