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
#ifndef _RANGE_CONTROL_H_
#define _RANGE_CONTROL_H_

#include "SliderControl.h"
#include <QRadioButton>
#include <QButtonGroup>

class RangeControl : public QWidget {
    Q_OBJECT
protected:
    SliderControl* sliderControl;
    QGridLayout* layout;
    QRadioButton* range1Button;
    QRadioButton* range2Button;
    QRadioButton* range3Button;
    QRadioButton* range4Button;
    QButtonGroup* buttonGroup;
    float multiplier;
    float rangeMultiplier;
private slots:
    void inputChanged(float value);
    void inputChangedDelayed(float value);
    void rangeChanged(int button);
signals:
    void valueChanged(float value);
    void valueChangedDelayed(float value);
public:
    RangeControl(QString captionFormat, float value);
    virtual ~RangeControl();
    float getValue();
};

#endif /* _RANGE_CONTROL_H_ */
