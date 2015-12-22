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
#ifndef _SLIDER_CONTROL_H_
#define _SLIDER_CONTROL_H_

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QGridLayout>
#include <QLineEdit>
#include "DataConverter.h"
#include <cmath>

class SliderControl : public QWidget {
    Q_OBJECT
private:
    QLabel* captionLabel;
    QLabel* unitLabel;
    QSlider* slider;
    QLineEdit* lineEdit;
    float value;
    float min,max;
    float step;
    int numDigits;
    QGridLayout* layout;
    QTimer* timer;

    void setupTimer();
public:
    SliderControl(QString captionFormat, float value, float min,
        float max,float step);
    virtual ~SliderControl();
    float getValue();
    void setUnitLabel(QString value);
    void setRange(float value, float min,float max,float step);
    void setValue(float value);
private slots:
    void inputValue(int value);
    void inputValue();
    void timerEvent();
signals:
    void valueChanged(float value);
    void valueChangedDelayed(float value);
};

#endif /* _SLIDER_CONTROL_H_ */
