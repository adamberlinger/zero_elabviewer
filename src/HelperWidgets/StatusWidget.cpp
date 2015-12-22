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
#include "StatusWidget.h"

StatusWidget::StatusWidget(DeviceDescription* deviceDescription){
    this->setLayout(layout = new QVBoxLayout(this));

    layout->addWidget(connectedStatusLabel = new QLabel("Device not connected"));
    layout->addWidget(deviceNameLabel = new QLabel("Device: unknown"));
    layout->addWidget(deviceConfigurationLabel = new QLabel("Configuration: default"));

    this->deviceDescription = deviceDescription;
    QObject::connect (deviceDescription, SIGNAL(valuesChanged()), this, SLOT(deviceDescriptionChanged()));
}

void StatusWidget::deviceConnected(){
    connectedStatusLabel->setText("Device connected");
}
void StatusWidget::deviceDisconnected(){
    connectedStatusLabel->setText("Device not connected");
    deviceNameLabel->setText("Device: unknown");
    deviceConfigurationLabel->setText("Configuration: default");
}

void StatusWidget::deviceDescriptionChanged(){
    deviceNameLabel->setText(QString("Device: %1").arg(deviceDescription->name));
    if(!deviceDescription->configurationName.isEmpty()){
        deviceConfigurationLabel->setText(QString("Configuration: %1").arg(deviceDescription->configurationName));
    }
    else {
        deviceConfigurationLabel->setText("Configuration: default");
    }
}

StatusWidget::~StatusWidget(){

}
