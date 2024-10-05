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
#ifndef _DATA_SET_H_
#define _DATA_SET_H_

#include <QtGui>
#include <QtQml>
#include "DataConverter.h"

#define DATASET_MAX_AVG    (32)
#define MAX_BUFFER_COUNT        (15)

struct BufferDescription {
    /** \brief totalWidth = channels * sampleWidth */ 
    int totalWidth;
    /** \brief Number of channels */
    int channels;
    /** \brief Size of sample in bytes */
    int sampleWidth;
    int dataType;
    int size;

    BufferDescription(){}
    /**
     * \brief Inits the structure from Buffer description byte
     * \param input Encoded buffer header
     */
    void init(char input){
        channels = ((input >> 4) & 0xF);
        sampleWidth = ((input >> 2) & 0x3);
        totalWidth = channels * sampleWidth;
        dataType = (input & 0x3);
    }
};

class DataStream : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif
public:
    enum DataType {
        DATA_ANALOG = 0,
        DATA_BITS = 1
    };

    union DataUnion {
        QVector<double> *doubleData;
        QVector<uint8_t> *bitData;
    };
protected:
    DataType dataType;
    DataUnion data;
    double currentDataOffset;
    double newDataOffset;
public:
    DataStream(DataType dataType, int size);
    DataStream(QVector<double>* data);
    DataStream(QVector<uint8_t>* data);
    QVector<double>* getDouble(){return data.doubleData;}
    QVector<uint8_t>* getBits(){return data.bitData;}
    Q_INVOKABLE double getDoubleValue(int i){return data.doubleData->at(i) - currentDataOffset;}
    Q_INVOKABLE int length(){return data.doubleData->length();}
    DataType getType(){return dataType;}
    void setOffset(double value) {newDataOffset = value;}
    void applyOffset(double value);
    void updateOffset() {currentDataOffset = newDataOffset;}
    void removeOffset();
    double getOffset() {return currentDataOffset;}
    void resize(int size);
    void clear();
    void add(const DataStream* other);
    void multiply(double value);
    virtual ~DataStream();
};

class DataSet : public QObject{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif
public:
    enum ApproxType {
        /** None */
        APPROX_NONE,
	/** Shift the X axis */
        APPROX_SHIFT,
	/** Recompute Y axis. Not implemented yet */
        APPROX_LINEAR,
    };

    enum OffsetCalculation {
        OFFSET_SPREAD = 0,
        OFFSET_NONE = 1,
    };
protected:
    /** \brief Vector storing the X axis (all channels are synchronous) */
    QVector<double> *xAxis;
    /** \brief Dynamic array storing Y axises of channels */
    DataStream **yAxises;
    /** \brief Number of channels */
    int channels;
    /** \brief Number of samples per channel */
    int size;
    /** \brief Sampling frequency */
    double fs;
    /** \brief Sub-sample offset of the trigger */
    double displayOffset;
    /** \brief X position of the trigger */
    double trigger_x;
    /** \brief Y position of the trigger */
    double trigger_y;
    /** \brief Approximation type */
    ApproxType approx;
    /** \brief Rule for calculating offset */
    OffsetCalculation offsetCalculation;
    /** \brief Bit field of enabled channels */
    uint32_t channel_mask;
    /** \brief Index of the trigger channel */
    int trigger_channel;
    int total_channels;
    /** \brief Number of waveforms to average */
    uint32_t averaging;
    /** \brief Current index for averaging */
    uint32_t avg_index;
    /** \brief Current number of samples for averaging (used during startup) */
    uint32_t avg_count;
    /** \brief Current number of Y buffers */
    int current_y_size;
    /** \brief Previous header, we need to reset averaging when parameters change */
    uint8_t previous_header[MAX_BUFFER_COUNT+1];
    /** \brief Previous transfer size (need to reset averaging) */
    int last_transfer_size;
public slots:
    void setChannelMask(uint32_t value);
    void setTriggerChannel(int value);
    void resetAverage();
public:
    DataSet(int channels, double fs = 1.0);
    double setOffsetCalculation(DataSet::OffsetCalculation type, double value);
    double dataInput(BinaryTransfer* transfer,DataConverter* converter);
    int length(){return channels;}
    void setOffset(int channel, double value);
    void setSamplingFrequency(double fs){this->fs = fs;}
    Q_INVOKABLE DataStream *getData(int i){return yAxises[i];}
    Q_INVOKABLE QVector<double> *getXAxis(){return xAxis;}
    void setTrigger(double x,double y) {trigger_x = x; trigger_y = y;}
    double getTriggerOffset(int index);
    double getTriggerChannelOffset();
    void setDisplayOffset(double value){displayOffset = value;}
    void setAveraging(uint32_t value);
    int getAvgSamples(){return avg_count;}
    int getAvgIndex(){
        return (avg_index == 0)?avg_count:avg_index; 
    }
    int getAveraging(){return averaging;}
    virtual ~DataSet();
private:
    void clearYAxises();
};

#endif /* _DATA_SET_H_ */
