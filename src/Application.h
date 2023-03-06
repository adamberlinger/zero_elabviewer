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
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <QtGui>
#include <QSerialPort>
#include <QPushButton>
#include "qcustomplot.h"
#include "Protocol.h"
#include "DataConverter.h"
#include "SliderControl.h"
#include "ScopeWidget.h"
#include "PWMWidget.h"
#include "VoltmeterWidget.h"
#include "PWMInputWidget.h"
#include "GeneratorWidget.h"
#include "StatusWidget.h"
#include "PulseCounterWidget.h"
#include "AboutWindow.h"
#include "HelpWindow.h"

/**
 * \brief Main application class
 */
class Application : public QMainWindow {
    Q_OBJECT
private:
    QPixmap logo_image;
    QAction* exitAction;
    PWMWidget* pwmWindow;
    VoltmeterWidget* voltmeterWindow;
    PWMInputWidget* pwmInputWidget;
    GeneratorWidget* generatorWidget;
    PulseCounterWidget* pulseCounterWidget;

    StatusWidget* statusWidget;

    ScopeWidget* scopeWidget;

    QMenu *fileMenu;
    QMenu *aboutMenu;
    QAction *helpAction;
    QAction *targetHelpAction;
    QAction *aboutAction;
    AboutWindow* aboutWindow;
    HelpWindow* helpWindow;
    HelpWindow* targetHelpWindow;

    QBoxLayout *mainLayout;;
    QBoxLayout *leftLayout;;
    QBoxLayout *rightLayout;
    QWidget *mainWidget;
    QWidget *leftPanel;
    QWidget *rightPanel;

    QListWidget* comPortList;
    QPushButton* connectButton;
    QPushButton* disconnectButton;
    QPushButton* refreshButton;
    QPushButton* bootloaderButton;
    QPushButton* configurationButton;

    int fw_index;
    QButtonGroup* functionWidgetButtons;
    QVector<QWidget*> fw_vector;
    QVector<int> functionIds;

    QSerialPort* serialPort;
    Protocol* protocol;

    DataConverter* dataConverter;
    bool isConnected;
    QLabel *logo;

    void initPlot();
protected:
    virtual void closeEvent(QCloseEvent *event);
    void addFunctionWidget(QWidget* widget,QString title,int functionId);
private slots:
    void quit();
    void refreshPorts();
    void connectPort();
    void disconnectPort();
    void readPort();
    void checkConnection();
    void enterBootloader();
    void showFunctionWidget(int index);
    void changeTargetName();
public:
    Application();
    virtual ~Application();
};

#endif /* _APPLICATION_H_ */
