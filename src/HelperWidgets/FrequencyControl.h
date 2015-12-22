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
#ifndef _FREQUENCY_CONTROL_H_
#define _FREQUENCY_CONTROL_H_

#include "SliderControl.h"
#include <QRadioButton>
#include <QButtonGroup>

class FrequencyControl : public QWidget {
    Q_OBJECT
protected:
    enum RangeType {
        RANGE_DEFAULT,
        RANGE_FINE
    };

    RangeType selectedRange;
    QSlider* slider;
    QGridLayout* layout;
    QRadioButton* range1Button;
    QRadioButton* range2Button;
    QButtonGroup* buttonGroup;
    QLabel* captionLabel;
    QLabel* unitLabel;
    QLineEdit* lineEdit;
    QTimer* timer;
    QLabel* realFrequency;

    float startRange;
    float endRange;
    float value;
    float realValue;
    float scale;
    bool changingValue;

    void setupTimer();

    float rangeIndexToFloat(int input);
    int floatToRangeIndex(float input);
private slots:
    void inputValue(int value);
    void inputValue();
    void rangeChanged(int button);
    void timerEvent();
signals:
    void valueChanged(float value);
    void valueChangedDelayed(float value);
public:
    FrequencyControl(QString caption, float value, bool showRealFrequency = false);
    virtual ~FrequencyControl();
    float getValue(){return realValue;}
    void setRealValue(float value);
};

#endif /* _FREQUENCY_CONTROL_H_ */
