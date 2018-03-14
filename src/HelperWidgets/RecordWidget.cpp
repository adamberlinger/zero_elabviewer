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

#define MAX_SAMPLES     (1000)

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

MultiRecordWidgetChannel::MultiRecordWidgetChannel(MultiRecordWidget* parent, ExtendedPlot* plot,
    int slot, bool visibility, QString name):QObject(parent){
    graph = plot->addGraph();
    toggle = new QAction(name, this);
    showOnly = new QAction(name, this);
    toggle->setCheckable(true);
    toggle->setChecked(visibility);
    graph->setVisible(visibility);

    this->slot = slot;
    this->parent = parent;
    offset = 0.0;

    QObject::connect(toggle, SIGNAL(triggered(bool)), this, SLOT(toggleVisibility(bool)));
    QObject::connect(showOnly, SIGNAL(triggered()), this, SLOT(setShowOnly()));
    graph->setPen(QPen(ExtendedPlot::graph_colors[slot % GRAPH_COLORS_SIZE]));
}

MultiRecordWidgetChannel::~MultiRecordWidgetChannel(){
    if(toggle){
        delete toggle;
    }
}


void MultiRecordWidgetChannel::setOffset(double value){
    double diff = value - offset;
    offset = value;
    for(int i = 0;i < dataY.size();++i){
        dataY[i] += diff;
    }
    graph->setData(dataX,dataY);
}

void MultiRecordWidgetChannel::toggleVisibility(bool value){
    graph->setVisible(value);
    parent->setChannelVisibility(slot, value);
    if(value != toggle->isChecked()){
        toggle->setChecked(value);
    }
}

void MultiRecordWidgetChannel::setShowOnly(){
    parent->hideAll();
    this->toggleVisibility(true);
}

void MultiRecordWidgetChannel::clearData(){
    dataX.clear();
    dataY.clear();
    graph->setData(dataX,dataY);
}

MultiRecordWidget::MultiRecordWidget(QString caption, QString yAxisLabel,
    bool hideOnClose,double recordWidth, uint32_t fixedChannels){
    this->setLayout(mainLayout = new QHBoxLayout());
    this->hideOnClose = hideOnClose;
    this->recordWidth = recordWidth;
    this->fixedChannels = fixedChannels;
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

    channelMenu = new QMenu(tr("Channels visibility"),plot);
    singleChannelMenu = new QMenu(tr("Show single channel"),plot);
    showAllAction = new QAction(tr("Show all channels"), plot);
    offsetChannelsAction = new QAction(tr("Offset channels"), plot);
    offsetChannelsAction->setCheckable(true);
    plot->getContextMenu()->addSeparator();
    plot->getContextMenu()->addMenu(channelMenu);
    plot->getContextMenu()->addMenu(singleChannelMenu);
    plot->getContextMenu()->addAction(showAllAction);
    plot->getContextMenu()->addAction(offsetChannelsAction);
    hideChannelMask = 0;

    QObject::connect (showAllAction, SIGNAL(triggered()), this, SLOT(showAll()));
    QObject::connect (offsetChannelsAction, SIGNAL(triggered(bool)), this, SLOT(enableOffset(bool)));

    if(fixedChannels > 0){
        for(int slot = 0;slot < fixedChannels;++slot){
            MultiRecordWidgetChannel* &activeChannel = channels[slot];
            if(activeChannel == NULL){
                bool visiblity =  (slot >= 0 && slot < 32)?(!(hideChannelMask & (1 << slot))):true;
                activeChannel = new MultiRecordWidgetChannel(this, plot, slot,visiblity, QString("Channel %1").arg(slot));
                //activeChannel->toggle->setText(Q);
                channelMenu->addAction(activeChannel->toggle);
                singleChannelMenu->addAction(activeChannel->showOnly);
            }
        }
    }
}

void MultiRecordWidget::hideAll(){
    for(auto it = channels.begin();it != channels.end();it++){
        it.value()->toggleVisibility(false);
    }
}

void MultiRecordWidget::showAll(){
    for(auto it = channels.begin();it != channels.end();it++){
        it.value()->toggleVisibility(true);
    }
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

void MultiRecordWidget::enableOffset(bool value){
    /* TODO: improve behaviour for dynamic number of channels */
    if(fixedChannels){
        double offset = 0.0;
        for(auto it = channels.begin();it != channels.end();it++){
            if(value){
                it.value()->setOffset(it.value()->slot * 4.0);
            }
            else {
                it.value()->setOffset(0.0);
            }
        }

        if(value){
            plot->lazyZoomY(-0.1, 4.0 * fixedChannels);
        }
        else {
            plot->lazyZoomY(min - 0.1, max + 0.1);
        }
        plot->replot();
    }
}

void MultiRecordWidget::clearRecord(){
    if(fixedChannels == 0){
        for(auto it = channels.begin();it != channels.end();it++){
            delete it.value();
        }
        channels.clear();
        plot->clearGraphs();
    }
    else {
        for(auto it = channels.begin();it != channels.end();it++){
            it.value()->clearData();
        }
    }
    plot->resetZoom(0.0,0.0,this->recordWidth,3.3);
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

void MultiRecordWidget::setChannelVisibility(int slot,bool value){
    if(slot < 32 && slot >= 0){
        uint32_t mask = (0x1) << slot;
        if(value){
            hideChannelMask &= ~mask;
        }
        else {
            hideChannelMask |= mask;
        }
    }
}

void MultiRecordWidget::record(float value, int slot){
    if(fixedChannels && (slot < 0 || slot >= fixedChannels))
        return;

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
    MultiRecordWidgetChannel* &activeChannel = channels[slot];
    if(activeChannel == NULL){
        bool visiblity =  (slot >= 0 && slot < 32)?(!(hideChannelMask & (1 << slot))):true;
        activeChannel = new MultiRecordWidgetChannel(this, plot, slot,visiblity, QString("Channel %1").arg(slot));
        //activeChannel->toggle->setText();
        channelMenu->addAction(activeChannel->toggle);
        singleChannelMenu->addAction(activeChannel->showOnly);
    }
    activeChannel->dataX.push_back(recordTime);
    activeChannel->dataY.push_back(value + activeChannel->offset);
    if(activeChannel->dataX.size() > (MAX_SAMPLES + 1000)){
        activeChannel->dataX.erase(activeChannel->dataX.begin(),activeChannel->dataX.begin()+1000);
        activeChannel->dataY.erase(activeChannel->dataY.begin(),activeChannel->dataY.begin()+1000);
    }
    activeChannel->graph->setData(activeChannel->dataX,activeChannel->dataY);
}

void MultiRecordWidget::recordSubmit(){
    recordActive = false;
    if(recordTime > this->recordWidth){
        plot->lazyZoomX(recordTime-this->recordWidth, recordTime);
    }
    if(fixedChannels == 0 || !offsetChannelsAction->isChecked()){
        plot->lazyZoomY(min - 0.1, max + 0.1);
    }
    else {
        plot->lazyZoomY(-0.1, 4.0 * fixedChannels + 0.1);
    }
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
