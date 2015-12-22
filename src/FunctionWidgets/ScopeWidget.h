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
#ifndef _SCOPE_WIDGET_H_
#define _SCOPE_WIDGET_H_

#include <QtGui>
#include <QtSerialPort/QtSerialPort>
#include <QCheckBox>
#include <QTabWidget>
#include "LiveJSScript.h"
#include "ExtendedPlot.h"
#include "Protocol.h"
#include "DataConverter.h"
#include "RangeControl.h"
#include "SignalMeasurement.h"
#include "RecordWidget.h"
#include "SpectralAnalysisWidget.h"
#include "DataAveraging.h"
#include "FrequencyControl.h"
#include "RunningIcon.h"
#include "ChannelControl.h"

class ScopeWidget : public QMainWindow {
    Q_OBJECT
protected:
    SignalMeasurement *signalMeasurement;

    QCPItemLine* vLineTrigger;
    QCPItemLine* hLineTrigger;

    ExtendedPlot* mainPlot;
    DataConverter* adcConverter;
    Protocol* protocol;
    int protocolChannel;
    QVector<double> *data_y;
    DataSet* dataSet;

    QWidget* controlPanel;

    SliderControl* triggerControl;
    FrequencyControl* sampleRateControl;
    SliderControl* triggerPositionControl;
    SliderControl* bufferSizeControl;
    QLabel* realBufferSizeLabel;
    QPushButton* triggerStart;
    QPushButton* triggerStop;
    QPushButton* resetZoomButton;
    QPushButton* showRecordButton;
    QPushButton* showSpectrumButton;

    QCheckBox* triggerRisingEdge;
    QCheckBox* triggerFallingEdge;
    QCheckBox* autoTrigger;
    QCheckBox* singleTrigger;

    ChannelControl* channelControl;

    QGridLayout* controlLayout;
    RunningIcon* runningIcon;
    QBoxLayout* measurementLayout;
    double plotWidth;
    unsigned int averaging;
    DataAveraging<double> signalAveraging;
    SliderControl* averagingControl;

    QWidget* measurementPanel;
    QLabel *averageLabel;
    QLabel *maxLabel;
    QLabel *minLabel;
    QLabel *noiseLabel;

    QWidget* statusPanel;
    QGridLayout* statusLayout;
    QLabel *maximumImpedanceLabel;

    RecordWidget* recordWidget;
    SpectralAnalysisWidget* spectralAnalysisWidget;

    QJSEngine* jsEngine;
    QJSValue jsDataSet;
    //QJSValue jsScript;
    LiveJSScript* liveJSScript;
    QCheckBox *jsEnableCheckBox;
    QTableWidget* jsResultTable;
    uint32_t timerFrequency;

    float getTriggerOffset(QVector<double>* data);
private slots:
    void startOsc();
    void stopOsc();
    void configureTrigger(float value);
    void repaintTrigger(float value);
    void configureSampleRate(float value);
    void configureTriggerPosition(float value);
    void configureTriggerMode(int value);
    void displayData();
    void configureTriggerPolarity(int unused);
    void configureAveraging(float value);
    void configureBufferSize(float value);
    void configureChannels(uint32_t channelMask);
    void configureTriggerChannel(int channel);
    void setChannelOffset(bool value);
    void openJSFile();
public slots:
    void configureAll();
    void commandReceived();
signals:
    void yieldAverageVoltage(float value);
public:
    ScopeWidget(Protocol* protocol, int channel, DataConverter* adcConverter);
    virtual ~ScopeWidget();
};

#endif /* _SCOPE_WIDGET_H_ */
