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
#ifndef _SPECTRAL_ANALYSIS_WIDGET_H_
#define _SPECTRAL_ANALYSIS_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include "SpectralAnalysis.h"
#include "ExtendedPlot.h"
#include "DataAveraging.h"
#include "RangeControl.h"

class SpectralAnalysisWidget : public QWidget {
    Q_OBJECT
protected:
    SpectralAnalysis *spectralAnalysis;
    ExtendedPlot* plot;
    QVector<double> *x;
    QVector<double> *y;
    QVector<double> *hx;
    QVector<double> *hy;

    QBoxLayout *mainLayout;
    QBoxLayout *sideLayout;
    QWidget *sidePanel;
    QCheckBox* windowCheckbox;
    QCheckBox* logScaleCheckbox;
    QComboBox* sizeSelect;
    QPushButton* resetZoomButton;
    QLabel* mainFrequencyLabel;
    double last_fs;
    bool useLogScale;
    int dataSize;
    int halfSize;

    DataAveraging<double> signalAveraging;
    SliderControl* averagingControl;

    void displayData(const double* values);
protected slots:
    void configureAveraging(float value);
public slots:
    void parametersChange();
    void logScaleToggled();
public:
    SpectralAnalysisWidget();
    void input(const double* samples,int size, double meanValue, double fs);
    
    static double convertToLog(double);
    static double convertFromLog(double);

    virtual ~SpectralAnalysisWidget();
};

#endif /* _SPECTRAL_ANALYSIS_WIDGET_H_ */
