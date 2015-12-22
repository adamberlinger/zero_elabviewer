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
#ifndef _DATACONVERTER_H_
#define _DATACONVERTER_H_

#include <stdint.h>
#include "Protocol.h"

class DataConverter : public QObject {
    Q_OBJECT
protected:
    int32_t source_min,source_max;
    float dest_min,dest_max;
    float dest_diff;
public slots:
    void setDestMax(double value);
public:
    DataConverter(int32_t source_min,int32_t source_max,float dest_min,float dest_max);

    uint32_t toDevice(float value);
    float fromDevice(int32_t value);
    QVector<double>* fromDevice16BitStream(BinaryTransfer* transfer);
    QVector<double>* fromDevice16BitStream(QVector<double>* in_result,const uint8_t* data, int size, int skip);

    static uint16_t convertSampleRate(float value);
    static const char* getUnitPrefix(double value,double* value_out);
    float getDestMax()const{return dest_max;}
};

#endif
