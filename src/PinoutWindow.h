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
#ifndef _PINOUT_WINDOW_H_
#define _PINOUT_WINDOW_H_

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QGridLayout>
#include "Pinout.h"

class PinoutWindow : public QWidget {
    Q_OBJECT
protected:
    QGridLayout* layout;
    QLabel** pinLabels;
    int pinCount;
    QLabel* title;
    QSpacerItem* spacer;
public:
    PinoutWindow();
    void clearPinout();
    void displayPinout(const Pinout& pinout);
    virtual ~PinoutWindow();
};

#endif

