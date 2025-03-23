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
#include "Application.h"
#include <iostream>
#include <QtQml>
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
#include <QtGlobal>
#else
#include <QtSystemDetection>
#endif

Q_DECLARE_METATYPE(QVector<double>)

Application::Application(){
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qmlRegisterType<DataSet>();
    qmlRegisterType<DataStream>();
#endif

    dataConverter = new DataConverter(0,4096,0.0f,3.3f);
    /* adcDataConverter is not modified by initial VDDA.
        VDDA is measured with each sample. */
    DataConverter *adcDataConverter = new DataConverter(0,4096,0.0,3.3);

    serialPort = NULL;
    exitAction = new QAction(tr("E&xit"), this);

    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAction);

    aboutMenu = menuBar()->addMenu(tr("&About"));
    aboutMenu->addAction(helpAction = new QAction(tr("&Help"),this));
    aboutMenu->addAction(targetHelpAction = new QAction(tr("&Target help"),this));
    aboutMenu->addAction(aboutAction = new QAction(tr("&About"),this));
    aboutWindow = new AboutWindow();
    helpWindow = new HelpWindow("qrc:/help/help.html");
    targetHelpWindow = new HelpWindow("qrc:/help/help_targets.html");

    this->setMinimumSize(300,200);

#ifdef APP_RC_VERSION
    setWindowTitle(QString("Zero eLab Viewer v%1.%2rc%3").arg(APP_MAJOR_VERSION).arg(APP_MINOR_VERSION).arg(APP_RC_VERSION));
#else
    setWindowTitle(QString("Zero eLab Viewer v%1.%2").arg(APP_MAJOR_VERSION).arg(APP_MINOR_VERSION));
#endif
    setWindowIcon(QIcon(":/icon.ico"));

    protocol = new Protocol(1024);

    this->setCentralWidget(mainWidget = new QWidget(this));
    mainWidget->setLayout(mainLayout = new QHBoxLayout());
    mainLayout->addWidget(leftPanel = new QWidget(this),1);
    mainLayout->addWidget(rightPanel = new QWidget(this),1);

    leftPanel->setLayout(leftLayout = new QVBoxLayout());
    rightPanel->setLayout(rightLayout = new QVBoxLayout());

    rightLayout->addWidget(new QLabel("COM port:"));
    rightLayout->addWidget(comPortList = new QListWidget());
    rightLayout->addWidget(refreshButton = new QPushButton("Refresh"));
    rightLayout->addWidget(connectButton = new QPushButton("Connect"));
    rightLayout->addWidget(disconnectButton = new QPushButton("Disconnect"));
    rightLayout->addWidget(bootloaderButton = new QPushButton("Enter bootloader"));
    rightLayout->addWidget(configurationButton = new QPushButton("Next configuration"));

    this->refreshPorts();

    functionWidgetButtons = new QButtonGroup(this);
    fw_index = 1;

    QObject::connect (refreshButton, SIGNAL(pressed()), this, SLOT(refreshPorts()));
    QObject::connect (connectButton, SIGNAL(pressed()), this, SLOT(connectPort()));
    QObject::connect (disconnectButton, SIGNAL(pressed()), this, SLOT(disconnectPort()));
    QObject::connect (configurationButton, SIGNAL(pressed()), protocol, SLOT(nextDeviceConfiguration()));
    QObject::connect (aboutAction, SIGNAL(triggered()), aboutWindow, SLOT(show()));
    QObject::connect (helpAction, SIGNAL(triggered()), helpWindow, SLOT(show()));
    QObject::connect (targetHelpAction, SIGNAL(triggered()), targetHelpWindow, SLOT(show()));
    QObject::connect (bootloaderButton, SIGNAL(pressed()), this, SLOT(enterBootloader()));

    scopeWidget = new ScopeWidget(protocol, 1, dataConverter);
    pwmWindow = new PWMWidget(protocol, 2);
    voltmeterWindow = new VoltmeterWidget(protocol,3,adcDataConverter);
    pwmInputWidget = new PWMInputWidget(protocol,4);
    generatorWidget = new GeneratorWidget(protocol,5);
    pulseCounterWidget = new PulseCounterWidget(protocol,6);

    voltmeterWindow->setGenerator(generatorWidget);

    this->addFunctionWidget(scopeWidget, "Oscilloscope",1);
    this->addFunctionWidget(generatorWidget,"Generator",5);
    this->addFunctionWidget(pwmWindow, "PWM output",2);
    this->addFunctionWidget(pwmInputWidget, "PWM input",4);
    this->addFunctionWidget(voltmeterWindow, "Volt meter",3);
    this->addFunctionWidget(pulseCounterWidget, "Pulse counter",6);

    leftLayout->addWidget(statusWidget = new StatusWidget(protocol->getDeviceDescription()));

    logo_image.load(":/img/ctu_logo_tiny_en.png");
    leftLayout->addWidget(logo = new QLabel(""));
    logo->setPixmap(logo_image);
    leftLayout->addStretch(1);


    isConnected = false;

    QObject::connect (protocol, SIGNAL(supplyVoltageValue(double)), dataConverter, SLOT(setDestMax(double)));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QObject::connect (functionWidgetButtons, SIGNAL(buttonPressed(int)), this, SLOT(showFunctionWidget(int)));
    QObject::connect (functionWidgetButtons, SIGNAL(buttonPressed(int)), this, SLOT(checkConnection()));
#else
    QObject::connect (functionWidgetButtons, SIGNAL(idPressed(int)), this, SLOT(showFunctionWidget(int)));
    QObject::connect (functionWidgetButtons, SIGNAL(idPressed(int)), this, SLOT(checkConnection()));
#endif
    QObject::connect (protocol->getDeviceDescription(), SIGNAL(valuesChanged()), this, SLOT(changeTargetName()));
}

void Application::changeTargetName(){
    targetHelpWindow->setAnchor(protocol->getDeviceDescription()->name);

    for(int i = 0;i < fw_vector.size(); ++i){
        bool enabled = protocol->getDeviceDescription()
            ->hasCapability((DeviceDescription::Capabilities)functionIds[i]);
        functionWidgetButtons->button(i+1)->setEnabled(enabled);
    }
}

void Application::addFunctionWidget(QWidget* widget,QString title,int functionId){
    QPushButton* tmpButton = new QPushButton(title,this);
    leftLayout->addWidget(tmpButton);
    widget->setWindowTitle(title);
    functionWidgetButtons->addButton(tmpButton, fw_index);
    fw_vector.append(widget);
    functionIds.append(functionId);
    fw_index++;
}

void Application::showFunctionWidget(int index){
    if((index - 1) < fw_vector.size() && index > 0){
        fw_vector[index - 1]->show();
        fw_vector[index - 1]->showNormal();
        fw_vector[index - 1]->activateWindow();
    }
}

void Application::checkConnection(){
    if(!isConnected){
        QMessageBox msgBox;
        msgBox.setText("No device connected. You won't be able to receive any data.");
        msgBox.exec();
    }
}

void Application::enterBootloader(){
    protocol->command('B',0,0);
};

void Application::connectPort(){
    QString portname;
    if(comPortList->currentItem()){
        portname = comPortList->currentItem()->text();
    }
    else {
        return;
    }

    if(serialPort)
        delete serialPort;
    qDebug() << "Port open: " << portname;


    serialPort = new QSerialPort(portname);
    if(serialPort->open(QIODevice::ReadWrite)){
        serialPort->setBaudRate(115200);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setTextModeEnabled(false);

        int dummyData = serialPort->bytesAvailable();
        std::cout << "Dummy data: " << dummyData << std::endl;
        serialPort->readAll();
        if(!serialPort->clear(QSerialPort::AllDirections)){
            std::cout << "Clear failed" << std::endl;
        }


        protocol->setDevice(serialPort);
        statusWidget->deviceConnected();
        isConnected = true;

        QObject::connect (serialPort, SIGNAL(readyRead()), this, SLOT(readPort()));
    }
    else {
        QString error = serialPort->errorString();
        qDebug() << error;
        QMessageBox msgBox;

        QString hint("");

        if (serialPort->error() == QSerialPort::PermissionError) {
#ifdef Q_OS_LINUX
            hint = "\n\nAdding your user account to the dialout or uucp user groups might fix this,"
                   "\nsee https://github.com/cvut-fel-sdi/zero_elabviewer#permission-denied-on-linux";
#endif
        }

        msgBox.setText("Unable to connect to COM port:\n"+error+hint);
        msgBox.exec();
    }
}

void Application::disconnectPort(){
    if(serialPort){
        delete serialPort;
        serialPort = NULL;
    }
    protocol->setDevice(NULL);
    statusWidget->deviceDisconnected();
}

void Application::readPort(){
    protocol->processData();
}

Application::~Application(){
    delete serialPort;
    delete pwmWindow;
    delete pwmInputWidget;
    delete voltmeterWindow;

    serialPort = nullptr;
    pwmWindow = nullptr;
    pwmInputWidget = nullptr;
    voltmeterWindow = nullptr;
}

void Application::closeEvent(QCloseEvent *event){
    this->quit();
}

void Application::refreshPorts(){
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    comPortList->clear();
    for(int i = 0;i < ports.size();++i){
        comPortList->addItem(ports[i].portName());
    }
}

void Application::quit(){
    qApp->quit();
}
