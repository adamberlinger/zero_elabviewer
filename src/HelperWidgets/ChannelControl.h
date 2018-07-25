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
#ifndef _CHANNEL_CONTROL_H_
#define _CHANNEL_CONTROL_H_

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QGridLayout>
#include <QCheckBox>
#include <QButtonGroup>

class ChannelControl : public QWidget {
    Q_OBJECT
protected:
    QCheckBox **channelEnable;
    QRadioButton **channelTrigger;
    QButtonGroup *triggerButtonGroup;

    QGridLayout* layout;

    int numChannels;

protected slots:
    void radioButtonEvent(int channel);
    void checkBoxEvent();
public:
    ChannelControl(int numChannels,uint32_t channelMask);
    uint32_t getActiveChannelMask();
    virtual ~ChannelControl();
signals:
    void changedTriggerChannel(int channel);
    void changedActiveChannels(uint32_t channelMask);
};

#endif /* _CHANNEL_CONTROL_H_ */
