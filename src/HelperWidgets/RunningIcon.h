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
#ifndef _RUNNING_ICON_H_
#define _RUNNING_ICON_H_

#include <QtGui>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QIcon>

class RunningIcon : public QWidget {
    Q_OBJECT
protected:
    static QPixmap *icons;
    static void initPixmaps();

    QTimer* timer;
    bool isActive;
    bool isRunning;
    int timeout;

    QLabel* iconLabel;
    QLabel* textLabel;
    QLayout* layout;
protected slots:
    void timeoutEvent();
public:
    RunningIcon(int timeout = 1000);
    virtual ~RunningIcon();
public slots:
    void ping();
    void start();
    void stop();
};

#endif
