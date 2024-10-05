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
#include "ScopeWidget.h"
#include <iostream>

ScopeWidget::ScopeWidget(Protocol* protocol, int channel, DataConverter* adcConverter):QMainWindow(),signalAveraging(0){
    this->protocol = protocol;
    this->adcConverter = adcConverter;
    this->data_y = NULL;
    this->averaging = 0;

    dataSet = new DataSet(2);

    this->protocolChannel = channel;

    QMenu* settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QAction* offsetChannelsAction = new QAction(tr("Offset channels"),this);
    QAction* openJSAction = new QAction(tr("Set JS script"),this);

    setTabPosition (Qt::AllDockWidgetAreas, QTabWidget::North);
    setDockOptions (/*QMainWindow::AnimatedDocks | */QMainWindow::AllowTabbedDocks | QMainWindow::ForceTabbedDocks);

    offsetChannelsAction->setCheckable(true);
    offsetChannelsAction->setChecked(true);
    settingsMenu->addAction(offsetChannelsAction);

    settingsMenu->addAction(openJSAction);

    this->mainPlot = new ExtendedPlot(this);


    this->setCentralWidget(this->mainPlot);

    measurementPanel = new QWidget(this);
    statusPanel = new QWidget(this);
    signalMeasurement = new SignalMeasurement();

    mainPlot->addGraph();
    mainPlot->xAxis->setLabel("Time (ms)");
    mainPlot->yAxis->setLabel("Voltage (V)");

    mainPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    mainPlot->setMinimumSize(400,300);

    vLineTrigger = new QCPItemLine(mainPlot);
    hLineTrigger = new QCPItemLine(mainPlot);

    vLineTrigger->setPen(QPen(Qt::red));
    hLineTrigger->setPen(QPen(Qt::red));

    hLineTrigger->setVisible(false);
    vLineTrigger->setVisible(false);

    plotWidth = 100;
    mainPlot->replot();

    QDockWidget *dock = new QDockWidget(tr("Control"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    controlPanel = new QWidget(this);
    controlPanel->setLayout(controlLayout = new QGridLayout());
    dock->setWidget(controlPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    QDockWidget *dock2 = new QDockWidget(tr("Measurement"), this);
    dock2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock2->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock2->setWidget(measurementPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock2);

    QDockWidget *dock3 = new QDockWidget(tr("Status"), this);
    dock3->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock3->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock3->setWidget(statusPanel);
    addDockWidget(Qt::RightDockWidgetArea, dock3);

    QMainWindow::tabifyDockWidget(dock, dock2);
    QMainWindow::tabifyDockWidget(dock2, dock3);

    triggerControl = new SliderControl(QString("Trigger: %1 V"),1.5f,0.0f,3.3f,0.001f);
    sampleRateControl = new FrequencyControl(QString("Sample rate:"),1000.0f,true);
    triggerPositionControl = new SliderControl(QString("Trigger position: %1 %"),15.0f,0.0f,100.0f,0.1f);
    bufferSizeControl = new SliderControl(QString("Buffer size: %1 #"),1024.0f,10.0f,16*1024.0f,1.0f);
    averagingWaveControl = new SliderControl(QString("Averaging: %1 #"), 1.0f, 1.0f, (double)DATASET_MAX_AVG, 1.0f);
    triggerStart = new QPushButton("Start");
    triggerStop = new QPushButton("Stop");

    int row = 0;
    controlLayout->addWidget(triggerControl,row++,0,1,2);
    controlLayout->addWidget(triggerRisingEdge = new QCheckBox("Rising edge"),row,0,1,1);
    controlLayout->addWidget(triggerFallingEdge = new QCheckBox("Falling edge"),row++,1,1,1);
    controlLayout->addWidget(autoTrigger = new QCheckBox("Auto trigger"),row,0,1,1);
    controlLayout->addWidget(singleTrigger = new QCheckBox("Single"),row++,1,1,1);
    controlLayout->addWidget(channelControl = new ChannelControl(4,0x1),row++,0,1,2);
    controlLayout->addWidget(triggerStart,row,0,1,1);
    controlLayout->addWidget(triggerStop,row++,1,1,1);
    controlLayout->addWidget(triggerPositionControl,row++,0,1,2);
    controlLayout->addWidget(sampleRateControl,row++,0,1,2);
    controlLayout->addWidget(bufferSizeControl,row++,0,1,2);
    controlLayout->addWidget(averagingWaveControl,row,0,1,1);
    controlLayout->addWidget(resetAvgButton = new QPushButton("Reset Average"),row++,1,1,1);
    controlLayout->addWidget(realAvgSizeLabel = new QLabel("No averaging"), row++,0,1,2);
    controlLayout->addWidget(resetZoomButton = new QPushButton("Reset Zoom"),row++,0,1,2);
    controlLayout->addWidget(runningIcon = new RunningIcon(1000),row,0,1,1);
    controlLayout->addWidget(realBufferSizeLabel = new QLabel("Samples: ?"), row++,1,1,1);

    triggerRisingEdge->setCheckState(Qt::Checked);

    controlLayout->addItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding),row++,0,1,2);

    averagingControl = new SliderControl(QString("Averaging: %1 samples"),0.0f,0.0f,16.0f,1.0f);

    measurementPanel->setLayout(measurementLayout = new QVBoxLayout());
    measurementLayout->addWidget(averageLabel = new QLabel("Average: 0.0 V"));
    measurementLayout->addWidget(maxLabel = new QLabel("Max: 0.0 V"));
    measurementLayout->addWidget(minLabel = new QLabel("Min: 0.0 V"));
    measurementLayout->addWidget(noiseLabel = new QLabel("Noise: 0.0 V"));
    measurementLayout->addWidget(averagingControl);
    measurementLayout->addWidget(showRecordButton = new QPushButton("Show recording"));
    measurementLayout->addWidget(showSpectrumButton = new QPushButton("Show spectrum"));
    measurementLayout->addWidget(jsEnableCheckBox = new QCheckBox("Enable JS"));
    measurementLayout->addWidget(jsResultTable = new QTableWidget(this));
    measurementLayout->addStretch(1);

    statusPanel->setLayout(statusLayout = new QGridLayout());
    statusLayout->addWidget(maximumImpedanceLabel = new QLabel("Maximum input impedance: ? Ohms"));

    jsResultTable->verticalHeader()->setVisible(false);
    jsResultTable->horizontalHeader()->setStretchLastSection(true);
    jsResultTable->horizontalHeader()->setVisible(false);

    QObject::connect (triggerControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureTrigger(float)));
    QObject::connect (bufferSizeControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureBufferSize(float)));
    QObject::connect (averagingWaveControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureWaveAveraging(float)));
    QObject::connect (triggerControl, SIGNAL(valueChanged(float)), this, SLOT(repaintTrigger(float)));
    QObject::connect (triggerStart, SIGNAL(pressed()), this, SLOT(startOsc()));
    QObject::connect (singleTrigger, SIGNAL(stateChanged(int)), this, SLOT(singleTriggerToggle()));
    QObject::connect (triggerStop, SIGNAL(pressed()), this, SLOT(stopOsc()));
    QObject::connect (triggerStart, SIGNAL(pressed()), runningIcon, SLOT(start()));
    QObject::connect (triggerStop, SIGNAL(pressed()), runningIcon, SLOT(stop()));
    QObject::connect (sampleRateControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureSampleRate(float)));
    QObject::connect (protocol, SIGNAL(binaryReceived()), this, SLOT(displayData()));

    QObject::connect (triggerRisingEdge, SIGNAL(stateChanged(int)), this, SLOT(configureTriggerPolarity(int)));
    QObject::connect (triggerFallingEdge, SIGNAL(stateChanged(int)), this, SLOT(configureTriggerPolarity(int)));
    QObject::connect (channelControl, SIGNAL(changedActiveChannels(uint32_t)), this, SLOT(configureChannels(uint32_t)));
    QObject::connect (channelControl, SIGNAL(changedTriggerChannel(int)), this, SLOT(configureTriggerChannel(int)));
    QObject::connect (channelControl, SIGNAL(changedActiveChannels(uint32_t)), dataSet, SLOT(setChannelMask(uint32_t)));
    QObject::connect (channelControl, SIGNAL(changedTriggerChannel(int)), dataSet, SLOT(setTriggerChannel(int)));

    QObject::connect (autoTrigger, SIGNAL(stateChanged(int)), this, SLOT(configureTriggerMode(int)));
    QObject::connect (triggerPositionControl, SIGNAL(valueChangedDelayed(float)), this, SLOT(configureTriggerPosition(float)));

    QObject::connect (protocol, SIGNAL(deviceReconnected()), this, SLOT(configureAll()));
    QObject::connect (protocol, SIGNAL(commandReceived()), this, SLOT(commandReceived()));
    QObject::connect (resetZoomButton, SIGNAL(pressed()), this->mainPlot, SLOT(resetZoomAndReplot()));
    QObject::connect (resetAvgButton, SIGNAL(pressed()), this->dataSet, SLOT(resetAverage()));

    recordWidget = new RecordWidget("Average voltage - oscilloscope","Voltage",true);
    recordWidget->setWindowTitle("Average Voltage");
    QObject::connect (this, SIGNAL(yieldAverageVoltage(float)), recordWidget, SLOT(record(float)));
    QObject::connect (showRecordButton, SIGNAL(pressed()), recordWidget, SLOT(show()));

    spectralAnalysisWidget = new SpectralAnalysisWidget();
    spectralAnalysisWidget->setWindowTitle("Oscilloscope Spectrum");
    QObject::connect (showSpectrumButton, SIGNAL(pressed()), spectralAnalysisWidget, SLOT(show()));

    QObject::connect (averagingControl, SIGNAL(valueChanged(float)), this, SLOT(configureAveraging(float)));

    jsEngine = new QJSEngine();

    liveJSScript = NULL;

    jsDataSet = jsEngine->newQObject(dataSet);
    jsResultTable->setColumnCount(2);
    timerFrequency = 0;

    QObject::connect (offsetChannelsAction, SIGNAL(toggled(bool)), this, SLOT(setChannelOffset(bool)));
    QObject::connect (openJSAction, SIGNAL(triggered()), this, SLOT(openJSFile()));

    dock->show();
    dock->raise();
    runState = STOPPED;
}

void ScopeWidget::singleTriggerToggle(){
    if(runState != STOPPED){
        startOsc();
    }
}

void ScopeWidget::commandReceived(){
    const Command* command = protocol->getLastCommand();
    if(command->channel == this->protocolChannel){
        if(command->command_id == 'F'){
            timerFrequency = command->value;
        }
        else if(command->command_id == 'D') {
            if(timerFrequency > 0){
                double realFrequency = ((double)timerFrequency) / command->value;
                sampleRateControl->setRealValue(realFrequency);
            }
        }
        else if(command->command_id == 'R') {
            double value;
            const char* unit = DataConverter::getUnitPrefix(command->value, &value);
            maximumImpedanceLabel->setText(QString("Maximum input impedance: %1 %2Ohms").arg(value).arg(unit));
        }
    }
}

void ScopeWidget::setChannelOffset(bool value){
    double maxHeight = this->dataSet->setOffsetCalculation(value?DataSet::OFFSET_SPREAD:DataSet::OFFSET_NONE,0.0);
    mainPlot->setDataSet(dataSet);
    if(maxHeight > 0.1){
        mainPlot->lazyZoomY(0,maxHeight);
    }
    mainPlot->replot();
}

void ScopeWidget::openJSFile(){
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Script"), QString(), tr("Script Files (*.js)"));
    if(!fileName.isNull()){
        if(liveJSScript != NULL){
            delete liveJSScript;
        }
        liveJSScript = new LiveJSScript(jsEngine, fileName);
    }
}

void ScopeWidget::configureAll(){
    configureTrigger(triggerControl->getValue());
    configureTriggerPosition(triggerPositionControl->getValue());
    configureSampleRate(sampleRateControl->getValue());
    configureTriggerPolarity(0);
    configureChannels(channelControl->getActiveChannelMask());
}

void ScopeWidget::configureChannels(uint32_t channelMask){
    protocol->command('C',this->protocolChannel,channelMask);
}

void ScopeWidget::configureTrigger(float value){
    uint16_t device_value = adcConverter->toDevice(value);
    protocol->command('T',this->protocolChannel,device_value);
}

void ScopeWidget::configureTriggerChannel(int channel){
    protocol->command('R',this->protocolChannel,(uint16_t)channel);
}

void ScopeWidget::configureBufferSize(float value){
    protocol->command('B',this->protocolChannel, (int)std::floor(value + 0.5f));
}

void ScopeWidget::configureWaveAveraging(float value){
    dataSet->setAveraging(std::floor(value + 0.5f));
}

void ScopeWidget::configureTriggerPosition(float value){
    uint16_t device_value = std::floor(value * 10.0f + 0.5f);
    protocol->command('D',this->protocolChannel,device_value);
}

void ScopeWidget::configureTriggerMode(int value){
    protocol->command('M',this->protocolChannel,value);
}

void ScopeWidget::configureAveraging(float value){
    int intValue = (int)value;
    if(this->averaging == 0 && intValue > 0){
        signalAveraging.reset();
    }

    signalAveraging.setAveraging(intValue);
    this->averaging = intValue;
}

void ScopeWidget::configureTriggerPolarity(int unused){
    char val = 0;
    (void)unused;
    if(triggerFallingEdge->checkState()) val |= 0x2;
    if(triggerRisingEdge->checkState()) val |= 0x1;
    protocol->command('P',this->protocolChannel,val);
}

void ScopeWidget::repaintTrigger(float value){

    hLineTrigger->start->setCoords(0,value);
    hLineTrigger->end->setCoords(plotWidth,value);

    mainPlot->replot();
}

void ScopeWidget::configureSampleRate(float value){
    uint16_t device_value = DataConverter::convertSampleRate(value);
    protocol->command('F',this->protocolChannel,(uint16_t)device_value);
    dataSet->resetAverage();
}

void ScopeWidget::startOsc(){
    if(singleTrigger->isChecked()){
        protocol->command('S',this->protocolChannel,2);
        runState = SINGLE;
    }
    else {
        protocol->command('S',this->protocolChannel,1);
        runState = RUNNING;
    }
}

void ScopeWidget::stopOsc(){
    protocol->command('S',this->protocolChannel,0);
    runState = STOPPED;
}

void ScopeWidget::displayData(){
    BinaryTransfer* transfer = protocol->popTransfer(this->protocolChannel);
    if(transfer != NULL){
        if(runState == SINGLE){
            runningIcon->stop();
            runState = STOPPED;
        }
        else {
            runningIcon->ping();
        }
        dataSet->setTrigger(triggerPositionControl->getValue()*0.01, triggerControl->getValue());

        float sampleTime = sampleRateControl->getValue() * 0.001f;

        dataSet->setSamplingFrequency(sampleTime);
        double maxHeight = dataSet->dataInput(transfer,adcConverter);
        dataSet->setOffset(1,4.0);
        mainPlot->setDataSet(dataSet);

        int size = dataSet->getXAxis()->length();
        int avg_size = dataSet->getAvgSamples();
        if(averagingWaveControl->getValue() > 1.5f){
            realAvgSizeLabel->setText(QString("Avg. samples %1/%3, (index %2)")
                .arg(avg_size).arg(dataSet->getAvgIndex())
                .arg(dataSet->getAveraging()));
        }
        else {
            realAvgSizeLabel->setText(QString("No averaging"));
        }
        realBufferSizeLabel->setText(QString("Samples: %1").arg(size));
        mainPlot->lazyZoomX(0,size / sampleTime);
        if(dataSet->length() > 1){
            mainPlot->lazyZoomY(0,maxHeight);
        }
        else{
            mainPlot->lazyZoomY(0,adcConverter->getDestMax());
        }

        double tx = std::floor(size * triggerPositionControl->getValue() * 0.01f)
                    / sampleTime;
        plotWidth = (size - 1) / (sampleTime);
        double ty_offset = dataSet->getTriggerChannelOffset();
        hLineTrigger->start->setCoords(0,triggerControl->getValue()+ty_offset);
        hLineTrigger->end->setCoords(plotWidth,triggerControl->getValue()+ty_offset);

        vLineTrigger->start->setCoords(tx,maxHeight);
        vLineTrigger->end->setCoords(tx,0.0);

        hLineTrigger->setVisible(true);
        vLineTrigger->setVisible(true);

        mainPlot->replot();

        signalMeasurement->input(dataSet->getData(0)->getDouble());

        /*TODO: set measurement labels */
        averageLabel->setText(QString("Average: %1 V").arg(signalMeasurement->average,7,'f',4));
        maxLabel->setText(QString("Max: %1 V").arg(signalMeasurement->maxValue,7,'f',4));
        minLabel->setText(QString("Min: %1 V").arg(signalMeasurement->minValue,7,'f',4));
        noiseLabel->setText(QString("Noise: %1 V").arg(signalMeasurement->noise,7,'f',4));

        yieldAverageVoltage(signalMeasurement->average);

        spectralAnalysisWidget->input(dataSet->getData(0)->getDouble()->constData(),dataSet->getData(0)->getDouble()->length(),signalMeasurement->average,sampleRateControl->getValue());

        if(jsEnableCheckBox->isChecked() && liveJSScript != NULL){
            QJSValueList args;
            args << jsDataSet;
            QJSValue result = liveJSScript->call(args);
            if(result.isError()){
                qDebug() << "Uncaught exception at line"
                    << result.property("lineNumber").toInt()
                    << ":" << result.toString();
            }
            else if(result.isArray()){
                int length = result.property("length").toInt();
                jsResultTable->clearContents();
                jsResultTable->setRowCount(length);
                for(int i = 0; i < length;++i){
                    jsResultTable->setItem(i,0,new QTableWidgetItem(
                        result.property(i).property("name").toString()));
                    jsResultTable->setItem(i,1,new QTableWidgetItem(
                        result.property(i).property("value").toString()));
                }
            }
        }
    }
}

float ScopeWidget::getTriggerOffset(QVector<double>* data){
    int size = data->length();
    int i = std::floor(size * triggerPositionControl->getValue() * 0.01f) - 1;
    if(i < 0) i = 0;
    if((i+1) >= data->length()){
        return 0.0f;
    }
    float y1 = (float)(*data)[i];
    float y2 = (float)(*data)[i+1];
    float ty = triggerControl->getValue();
    bool triggerValid = false;
    triggerValid |= ((ty >= y1) && (ty <= y2));
    triggerValid |= ((ty >= y2) && (ty <= y1));
    if(triggerValid)
        return 1.0f - (ty - y1) / (y2 - y1);
    else {
        return 0.0f;
    }
}

ScopeWidget::~ScopeWidget(){
    delete recordWidget;
}
