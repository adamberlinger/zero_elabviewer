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
#ifndef _STATUS_WIDGET_H_
#define _STATUS_WIDGET_H_

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QBoxLayout>
#include "Protocol.h"

class StatusWidget : public QWidget {
    Q_OBJECT
protected:
    QBoxLayout* layout;

    QLabel* deviceNameLabel;
    QLabel* connectedStatusLabel;
    QLabel* deviceConfigurationLabel;
    QLabel* voltageLabel;
    DeviceDescription* deviceDescription;
public slots:
    void deviceConnected();
    void deviceDisconnected();
    void deviceDescriptionChanged();
public:
    StatusWidget(DeviceDescription* deviceDescription);
    virtual ~StatusWidget();
};

#endif /* _STATUS_WIDGET_H_ */
