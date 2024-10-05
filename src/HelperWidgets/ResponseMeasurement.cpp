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
#include "ResponseMeasurement.h"
#include <iostream>

ResponseMeasurement::ResponseMeasurement(QString caption, QString xAxisLabel, QString yAxisLabel,
    QString xUnits,QString yUnits){

    this->setLayout(mainLayout = new QHBoxLayout());
    plot = new ExtendedPlot(this);

    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, caption));
    plot->xAxis->setLabel(xAxisLabel);
    plot->yAxis->setLabel(yAxisLabel);
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->setMinimumSize(400,300);
    plot->resetZoom(0.0-0.1,-3.3 -0.1,3.3+0.1,3.3+0.1);
    plot->addGraph();

    mainLayout->addWidget(plot,1);
    mainLayout->addWidget(sidePanel = new QWidget(this),0);

    sidePanel->setLayout(controlLayout = new QVBoxLayout());
    controlLayout->addWidget(delayControl = new SliderControl("Delay %1 ms",20,1,1000,1));
    controlLayout->addWidget(stepsControl = new SliderControl("Steps: %1",100,2,1000,1));
    controlLayout->addWidget(startValueControl = new SliderControl("Start voltage %1 V",0,0,3.3,0.001));
    controlLayout->addWidget(stopValueControl = new SliderControl("Stop voltage %1 V",3.3,0,3.3,0.001));
    controlLayout->addWidget(startButton = new QPushButton("Start"));

    controlLayout->addStretch(1);

    QObject::connect (startButton, SIGNAL(pressed()), this, SLOT(startDC()));

    timer = new QTimer();
    timer->setSingleShot(true);
    QObject::connect (timer, SIGNAL(timeout()), this, SLOT(step()));

    xValues = yValues = NULL;
}

void ResponseMeasurement::startDC(){
    this->startSignal();
    currentOutput = startValueControl->getValue();
    this->numSteps = (int)stepsControl->getValue();
    this->outputStep = (stopValueControl->getValue() - currentOutput) / (this->numSteps - 1);
    this->index = 0;

    if(xValues){
        delete xValues;
    }
    if(yValues){
        delete yValues;
    }
    xValues = new QVector<double>(this->numSteps);
    yValues = new QVector<double>(this->numSteps);
    timer->start(100);


    (*xValues)[index] = currentOutput;
    this->output(currentOutput);
}

void ResponseMeasurement::step(){
    bool done = false;
    if(this->numSteps){
        this->numSteps--;
        done = true;
    }
    if(this->numSteps){
        timer->start((int)delayControl->getValue());
        currentOutput+=outputStep;
        (*xValues)[index+1] = currentOutput;
        this->output(currentOutput);
    }
    /* TODO: rewrite this !!!! */
    (*yValues)[index] = (double)readedValue;
    index++;

    if(done){
        plot->graph(0)->setData(*xValues,*yValues);
        plot->replot();
    }
}

void ResponseMeasurement::input(float value){
    this->readedValue = value;
}

ResponseMeasurement::~ResponseMeasurement(){

}
