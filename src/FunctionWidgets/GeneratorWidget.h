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
#ifndef _GENERATOR_WIDGET_H_
#define _GENERATOR_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include <QPushButton>
#include "Protocol.h"
#include "RangeControl.h"
#include "FrequencyControl.h"

class GeneratorWidget : public QWidget {
    Q_OBJECT
protected:
    Protocol* protocol;
    int protocolChannel;
    QBoxLayout* mainLayout;

    FrequencyControl* frequencyControl;
    SliderControl* scaleControl;
    SliderControl* offsetControl;
    SliderControl* voltageControl;
    QCheckBox* voltageOutputEnable;

    QRadioButton* shapeSineButton;
    QRadioButton* shapeTriangleButton;
    QRadioButton* shapeSawButton;
    QRadioButton* shapeSquareButton;
    QRadioButton* shapeNoiseButton;
    QButtonGroup* buttonGroup;

    QPushButton* startButton;
    QPushButton* stopButton;
    uint32_t timerFrequency;
public slots:
    void configureAll();
    void configureFrequency(float value);
    void configureScale(float value);
    void configureOffset(float value);
    void configureShape(int button_id);
    void configureVoltage();
    void startGenerator();
    void stopGenerator();
public slots:
    void setStaticVoltage(float value);
    void commandReceived();
public:
    GeneratorWidget(Protocol* protocol, int channel);
    virtual ~GeneratorWidget();
};

#endif /* _GENERATOR_WIDGET_H_ */
