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
#include "RecordWidget.h"

#define MAX_SAMPLES     (10000)

RecordWidget::RecordWidget(QString caption, QString yAxisLabel, bool hideOnClose,double recordWidth){
    this->setLayout(mainLayout = new QHBoxLayout());
    this->hideOnClose = hideOnClose;
    this->recordWidth = recordWidth;
    plot = new ExtendedPlot(this);

    plot->addGraph();
    plot->xAxis->setLabel("Time (s)");
    plot->yAxis->setLabel(yAxisLabel);

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->setMinimumSize(400,300);

    plot->lazyZoomX(0, this->recordWidth);
    plot->lazyZoomY(0, 3.3);

    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, caption));

    plot->setFrequencyMultiplier(1.0);
    plot->xCursorLabel = "%{x'} %{u}s";
    plot->xCursorSpaceLabel = "f = %{f'} %{uf}Hz\nΔt = %{x'} %{u}s";

    plot->setShowPoints(true);
    plot->replot();

    sidePanel = new QWidget(this);
    sidePanel->setLayout(controlLayout = new QVBoxLayout);
    controlLayout->addWidget(startButton = new QPushButton("Start"));
    controlLayout->addWidget(stopButton = new QPushButton("Stop"));
    controlLayout->addWidget(clearButton = new QPushButton("Clear"));


    controlLayout->addWidget(noiseLevelLabel = new QLabel("Noise:"));
    controlLayout->addWidget(noiseLevelButton = new QPushButton("Compute noise"));

    controlLayout->addStretch(1);

    mainLayout->addWidget(plot,1);
    mainLayout->addWidget(sidePanel,0);

    running = false;

    x = 0.0;
    min = 0;
    max = 3.3;

    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startRecording()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopRecording()));
    QObject::connect (clearButton, SIGNAL(pressed()), this, SLOT(clearRecord()));
    QObject::connect (noiseLevelButton, SIGNAL(pressed()), this, SLOT(computeNoise()));
}

void RecordWidget::computeNoise(){
    if(dataY.length() < 2)
        return;

    /* Kahan algorithm for sumation */

    double new_sum,error,y;

    double avg = 0.0f;
    error = 0.0f;
    for(int i = 0;i < dataY.length();++i){
        y = dataY[i] - error;
        new_sum = avg + y;
        error = (new_sum - avg) - y;
        avg = new_sum;
    }
    avg /= dataY.length();

    double noise_level = 0.0f;
    error = 0.0f;
    for(int i = 0;i < dataY.length();++i){
        double d = (dataY[i] - avg);

        y = (d*d) - error;
        new_sum = noise_level + y;
        error = (new_sum - noise_level) - y;
        noise_level = new_sum;
    }

    /* Unbiased estimate */
    noise_level /= (dataY.length() - 1);
    noise_level = sqrt(noise_level);
    noiseLevelLabel->setText(QString("Noise: %1").arg(noise_level));
}

void RecordWidget::startRecording(){
    running = true;
}
void RecordWidget::stopRecording(){
    running = false;
}
void RecordWidget::clearRecord(){
    dataX.clear();
    dataY.clear();
    plot->resetZoom(0.0,0.0,100.0,3.3);
    plot->graph(0)->setData(dataX,dataY);
    plot->removeAllCursors();
    plot->replot();
    x = 0.0;
    min = 0;
    max = 3.3;
}
void RecordWidget::record(float value){
    this->record(value,1.0f);
}


void RecordWidget::record(float value, float time){
    if(!running)
        return;

    if(x < 0.0001){
        min = max = value;
    }
    else {
        if(min > value) min = value;
        if(max < value) max = value;
    }
    dataX.push_back(x);
    dataY.push_back(value);
    if(dataX.size() > (MAX_SAMPLES + 1000)){
        dataX.erase(dataX.begin(),dataX.begin()+1000);
        dataY.erase(dataY.begin(),dataY.begin()+1000);
    }
    plot->graph(0)->setData(dataX,dataY);
    if(x > this->recordWidth){
        plot->lazyZoomX(x-this->recordWidth, x);
    }
    plot->lazyZoomY(min - 0.1, max + 0.1);
    plot->replot();

    x += time;
}

void RecordWidget::closeEvent(QCloseEvent *event){
    if(hideOnClose){
        event->ignore();
        this->hide();
    }
}

RecordWidget::~RecordWidget(){
}

MultiRecordWidget::MultiRecordWidget(QString caption, QString yAxisLabel, bool hideOnClose,double recordWidth){
    this->setLayout(mainLayout = new QHBoxLayout());
    this->hideOnClose = hideOnClose;
    this->recordWidth = recordWidth;
    plot = new ExtendedPlot(this);

    plot->xAxis->setLabel("Time (s)");
    plot->yAxis->setLabel(yAxisLabel);

    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->setMinimumSize(400,300);

    plot->lazyZoomX(0, this->recordWidth);
    plot->lazyZoomY(0, 3.3);

    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPPlotTitle(plot, caption));

    plot->setFrequencyMultiplier(1.0);
    plot->xCursorLabel = "%{x'} %{u}s";
    plot->xCursorSpaceLabel = "f = %{f'} %{uf}Hz\nΔt = %{x'} %{u}s";

    plot->setShowPoints(true);
    plot->replot();

    sidePanel = new QWidget(this);
    sidePanel->setLayout(controlLayout = new QVBoxLayout);
    controlLayout->addWidget(startButton = new QPushButton("Start"));
    controlLayout->addWidget(stopButton = new QPushButton("Stop"));
    controlLayout->addWidget(clearButton = new QPushButton("Clear"));


    controlLayout->addWidget(noiseLevelLabel = new QLabel("Noise:"));
    controlLayout->addWidget(noiseLevelButton = new QPushButton("Compute noise"));

    controlLayout->addStretch(1);

    mainLayout->addWidget(plot,1);
    mainLayout->addWidget(sidePanel,0);

    running = false;

    min = 0;
    max = 3.3;
    recordTime = -1.0;
    recordActive = false;
    minMaxInitialized = false;

    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startRecording()));
    QObject::connect (stopButton, SIGNAL(pressed()), this, SLOT(stopRecording()));
    QObject::connect (clearButton, SIGNAL(pressed()), this, SLOT(clearRecord()));
    QObject::connect (noiseLevelButton, SIGNAL(pressed()), this, SLOT(computeNoise()));
}

void MultiRecordWidget::computeNoise(){
    /* TODO: add channel selection */
#if 0
    if(dataY.length() < 2)
        return;

    /* Kahan algorithm for sumation */

    double new_sum,error,y;

    double avg = 0.0f;
    error = 0.0f;
    for(int i = 0;i < dataY.length();++i){
        y = dataY[i] - error;
        new_sum = avg + y;
        error = (new_sum - avg) - y;
        avg = new_sum;
    }
    avg /= dataY.length();

    double noise_level = 0.0f;
    error = 0.0f;
    for(int i = 0;i < dataY.length();++i){
        double d = (dataY[i] - avg);

        y = (d*d) - error;
        new_sum = noise_level + y;
        error = (new_sum - noise_level) - y;
        noise_level = new_sum;
    }

    /* Unbiased estimate */
    noise_level /= (dataY.length() - 1);
    noise_level = sqrt(noise_level);
    noiseLevelLabel->setText(QString("Noise: %1").arg(noise_level));
#endif
}

void MultiRecordWidget::startRecording(){
    running = true;
}
void MultiRecordWidget::stopRecording(){
    running = false;
}
void MultiRecordWidget::clearRecord(){
    channels.clear();
    plot->clearGraphs();
    plot->resetZoom(0.0,0.0,100.0,3.3);
    plot->removeAllCursors();
    plot->replot();
    min = 0;
    max = 3.3;
    recordTime = -1.0;
    minMaxInitialized = false;
}

void MultiRecordWidget::recordPrepare(){
    if(running){
        if(recordTime < 0.0){
            /* Record empty */
            timer.start();
            recordTime = 0.0;
        }
        else {
            recordTime += timer.restart() * 0.001;
        }
        recordActive = true;
    }
}

void MultiRecordWidget::recordPrepare(float time){
    if(running){
        if(recordTime < 0.0){
            timer.start();
            recordTime = 0.0;
        }
        else {
            recordTime += time;
            timer.restart();
        }
        recordActive = true;
    }
}

void MultiRecordWidget::record(float value, int slot){
    if(!running)
        return;

    if(!minMaxInitialized){
        min = max = value;
        minMaxInitialized = true;
    }
    else {
        if(min > value) min = value;
        if(max < value) max = value;
    }

    /* Intentional creation if slot doesn't exist */
    Channel &activeChannel = channels[slot];
    if(activeChannel.graph == NULL){
        activeChannel.graph = plot->addGraph();
    }
    activeChannel.dataX.push_back(recordTime);
    activeChannel.dataY.push_back(value);
    if(activeChannel.dataX.size() > (MAX_SAMPLES + 1000)){
        activeChannel.dataX.erase(activeChannel.dataX.begin(),activeChannel.dataX.begin()+1000);
        activeChannel.dataY.erase(activeChannel.dataY.begin(),activeChannel.dataY.begin()+1000);
    }
    activeChannel.graph->setData(activeChannel.dataX,activeChannel.dataY);
}

void MultiRecordWidget::recordSubmit(){
    recordActive = false;
    if(recordTime > this->recordWidth){
        plot->lazyZoomX(recordTime-this->recordWidth, recordTime);
    }
    plot->lazyZoomY(min - 0.1, max + 0.1);
    plot->replot();
}

void MultiRecordWidget::recordSimple(float value){
    this->recordPrepare();
    this->record(value, 0);
    this->recordSubmit();
}

void MultiRecordWidget::closeEvent(QCloseEvent *event){
    if(hideOnClose){
        event->ignore();
        this->hide();
    }
}

MultiRecordWidget::~MultiRecordWidget(){
}
