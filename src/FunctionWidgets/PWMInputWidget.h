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
#ifndef _PWM_INPUT_WIDGET_H_
#define _PWM_INPUT_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include <QPushButton>
#include "Protocol.h"
#include "RangeControl.h"
#include "RunningIcon.h"

class PWMInputWidget : public QWidget {
    Q_OBJECT
protected:
    Protocol* protocol;
    int protocolChannel;
    QBoxLayout* mainLayout;
    QLabel* frequencyLabel;
    QLabel* dutyCycleLabel;

    QPushButton* startButton;
    QPushButton* stopButton;
    RunningIcon* runningIcon;
private slots:
    void configureAll();
    void startPWM();
    void displayData();
    void stopPWM();
signals:
    void yieldFrequency(float value);
public:
    PWMInputWidget(Protocol* protocol, int channel);
    virtual ~PWMInputWidget();
};

#endif /* _PWM_INPUT_WIDGET_H_ */
