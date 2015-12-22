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
#ifndef _PWM_WIDGET_H_
#define _PWM_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include <QPushButton>
#include "Protocol.h"
#include "FrequencyControl.h"

class PWMWidget : public QWidget {
    Q_OBJECT
protected:
    Protocol* protocol;
    int protocolChannel;
    QBoxLayout* mainLayout;

    FrequencyControl* frequencyControl;
    SliderControl* dutyCycleControl;

    QPushButton* startButton;
    QPushButton* stopButton;
    uint32_t timerFrequency;
private slots:
    void configureAll();
    void configureFrequency(float value);
    void configureDutyCycle(float value);
    void startPWM();
    void stopPWM();
public slots:
    void commandReceived();
public:
    PWMWidget(Protocol* protocol, int channel);
    virtual ~PWMWidget();
};

#endif /* _PWM_WIDGET_H_ */
