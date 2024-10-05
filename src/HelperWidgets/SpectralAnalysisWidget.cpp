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
#include "SpectralAnalysisWidget.h"
#include <cmath>
#include <limits>

static int dataSizeSelectionValues[6] = {
    512,1024,2048,4096,
    16384,65536
};

SpectralAnalysisWidget::SpectralAnalysisWidget():signalAveraging(1){
    dataSize = 512;
    halfSize = (dataSize / 2) + 1;
    spectralAnalysis = new SpectralAnalysis(dataSize);

    this->setLayout(mainLayout = new QHBoxLayout());

    plot = new ExtendedPlot(this);
    plot->xAxis->setLabel("Frequency (Hz)");
    plot->yAxis->setLabel("X");
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    plot->setMinimumSize(400,300);
    plot->resetZoom(0.0,100.0,0.0,1.0);
    plot->addGraph();

    plot->xCursorLabel = "%{x'} %{u}Hz";
    plot->xCursorSpaceLabel = "Δf = %{x'} %{u}Hz";
    plot->yCursorLabel = "%{y}";
    plot->yCursorSpaceLabel = "ΔX = %{y}";
    plot->setFrequencyMultiplier(1.0);

    plot->plotLayout()->insertRow(0);
    plot->plotLayout()->addElement(0, 0, new QCPTextElement(plot, "Spectrum / FFT"));

    x = new QVector<double>(dataSize);
    y = new QVector<double>(dataSize);
    hx = new QVector<double>(halfSize);
    hy = new QVector<double>(halfSize);
    this->last_fs = 100;
    this->useLogScale = false;

    mainLayout->addWidget(plot,1);
    mainLayout->addWidget(sidePanel = new QWidget(this),0);

    sizeSelect = new QComboBox();
    for(int i = 0;i < 6; i++){
        sizeSelect->addItem(QString::number(dataSizeSelectionValues[i]));
    }
    averagingControl = new SliderControl(QString("Averaging: %1 samples"),1.0f,1.0f,16.0f,1.0f);

    sidePanel->setLayout(sideLayout = new QVBoxLayout());
    sideLayout->addWidget(new QLabel("Number of samples:"));
    sideLayout->addWidget(sizeSelect);
    sideLayout->addWidget(windowCheckbox = new QCheckBox("Hamming window"));
    sideLayout->addWidget(logScaleCheckbox = new QCheckBox("Logarithmic scale"));
    sideLayout->addWidget(resetZoomButton = new QPushButton("Reset zoom"));
    sideLayout->addWidget(averagingControl);
    sideLayout->addWidget(mainFrequencyLabel = new QLabel("Main frequency:"));
    sideLayout->addStretch(1);
    windowCheckbox->setChecked(true);

    QObject::connect (windowCheckbox, SIGNAL(stateChanged(int)), this, SLOT(parametersChange()));
    QObject::connect (logScaleCheckbox, SIGNAL(stateChanged(int)), this, SLOT(parametersChange()));
    QObject::connect (logScaleCheckbox, SIGNAL(stateChanged(int)), this, SLOT(logScaleToggled()));
    QObject::connect (resetZoomButton, SIGNAL(pressed()), plot, SLOT(resetZoomAndReplot()));
    QObject::connect (sizeSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(parametersChange()));

    QObject::connect (averagingControl, SIGNAL(valueChanged(float)), this, SLOT(configureAveraging(float)));
}

void SpectralAnalysisWidget::logScaleToggled(){
    if(logScaleCheckbox->isChecked()){
        plot->yCursorLabel = "%{y} dB";
        plot->yCursorSpaceLabel = "ΔX = %{y} dB";
        plot->updateCursorLabels(NULL,convertToLog);
    }
    else {
        plot->yCursorLabel = "%{y}";
        plot->yCursorSpaceLabel = "ΔX = %{y}";
        plot->updateCursorLabels(NULL,convertFromLog);
    }
}


double SpectralAnalysisWidget::convertToLog(double x){
    return 10 * std::log10(x + 0.01);
}

double SpectralAnalysisWidget::convertFromLog(double x){
    return std::pow(10.0,x * 0.1) - 0.01;
}

void SpectralAnalysisWidget::parametersChange(){
    int sizeIndex = sizeSelect->currentIndex();
    bool resize = false;
    if(dataSize != dataSizeSelectionValues[sizeIndex]){
        spectralAnalysis->resize(dataSizeSelectionValues[sizeIndex]);
        dataSize = dataSizeSelectionValues[sizeIndex];
        halfSize = (dataSize / 2) + 1;

        x->resize(dataSize);
        y->resize(dataSize);
        hx->resize(halfSize);
        hy->resize(halfSize);
        resize = true;
    }

    spectralAnalysis->enableWindow(windowCheckbox->isChecked());

    this->useLogScale = logScaleCheckbox->isChecked();

    if(this->useLogScale){
        plot->yAxis->setLabel("X (dB)");
    }
    else {
        plot->yAxis->setLabel("X");
    }

    const double* result = spectralAnalysis->recompute();

    if(resize){
        displayData(result);
    }
    else {
        displayData(NULL);
    }
}

void SpectralAnalysisWidget::input(const double* samples,int size, double meanValue, double fs){
    const double* result = spectralAnalysis->compute(samples,size,meanValue);

    this->last_fs = fs;

    displayData(result);
}


void SpectralAnalysisWidget::configureAveraging(float value){
    int intValue = (int)value;

    signalAveraging.setAveraging(intValue);
}

void SpectralAnalysisWidget::displayData(const double* values){
    double max = 0.0;
    double min = std::numeric_limits<double>::max();
    double tmp;
    double f_step =  (last_fs / dataSize);
    QVector<double>* result;
    QVector<double>* xAxis;
    QVector<double>* yAxis;
    int maxIndex = -1;

    if(values){
        for(int i = 0;i < halfSize;++i){
            (*hx)[i] = i * f_step;
            (*hy)[i] = values[i];
        }

        signalAveraging.commit(hy);
    }

    xAxis = hx;
    yAxis = hy;
    plot->lazyZoomX(0,last_fs / 2);

    result = signalAveraging.getResult();

    for(int i = 0;i < result->length();++i){
        tmp = (*result)[i];
        if(useLogScale){
            tmp = 10 * std::log10(tmp + 0.01);
        }
        (*yAxis)[i] = tmp;
        if(max < tmp){
            max = tmp;
            maxIndex = i;
        }
        else if(min > tmp) min = tmp;
    }

    plot->graph(0)->setData(*xAxis,*yAxis);

    if(useLogScale){
        plot->lazyZoomY(std::max(max - 100,min),max);
    }
    else {
        plot->lazyZoomY(0,max);
    }
    plot->replot();

    double mainFrequency = (maxIndex * last_fs) / dataSize;
    double mainFrequencyScaled;
    const char* unitPrefix = DataConverter::getUnitPrefix(mainFrequency,&mainFrequencyScaled);

    mainFrequencyLabel->setText(QString("Main frequency: %1 %3Hz\n   ± %2")
        .arg(mainFrequencyScaled)
        .arg((last_fs / dataSize) * (mainFrequencyScaled / mainFrequency))
        .arg(unitPrefix));
}

SpectralAnalysisWidget::~SpectralAnalysisWidget(){

}
