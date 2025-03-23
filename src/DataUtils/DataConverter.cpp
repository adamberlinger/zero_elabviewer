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
#include "DataConverter.h"

DataConverter::DataConverter(int32_t source_min,int32_t source_max,
    float dest_min,float dest_max){
    this->source_min = source_min;
    this->source_max = source_max;
    this->dest_min = dest_min;
    this->dest_max = dest_max;

    this->dest_diff = (dest_max - dest_min);
}

void DataConverter::setDestMax(double value){
    this->dest_max = value;
    this->dest_diff = (dest_max - dest_min);
}

uint32_t DataConverter::toDevice(float value){
    return ((value - dest_min) / dest_max) * (source_max - source_min) + source_min;
}

float DataConverter::fromDevice(int32_t value){
    return ((value - source_min) / (float)source_max) * dest_diff + dest_min;
}

QVector<double>* DataConverter::fromDevice16BitStream(BinaryTransfer* transfer){

    int size = transfer->getSize() / 2;
    const int16_t *data = (const int16_t*)transfer->getData();
    QVector<double> *result = new QVector<double>(size);

    for(int i = 0;i < size;++i){
        (*result)[i] = this->fromDevice(data[i]);
    }

    return result;
}

QVector<double>* DataConverter::fromDevice16BitStream(QVector<double>* in_result,const uint8_t* data, int size, int step){
    int out_size = size / step;
    QVector<double> *result;
    if(in_result){
        in_result->resize(out_size);
        result = in_result;
    }
    else {
        result = new QVector<double>(out_size);
    }

    for(int i = 0,i_out = 0;i < size;i+=step,i_out++){
        uint16_t out_sample = (data[i] & 0xFF);
        out_sample |= (data[i+1] << 8);
        (*result)[i_out] = this->fromDevice(out_sample);
    }
    return result;
}

uint16_t DataConverter::convertSampleRate(float value){
    uint16_t device_value = 0;
    if(value < 1e4){
        device_value = (uint16_t)value;
    }
    else if(value < 1e7){
        device_value = (0x1 << 14) | (((uint16_t)(value * 1e-3)) & 0x3FFF);
    }
    else if(value < 1e9){
        device_value = (0x2 << 14) | (((uint16_t)(value * 1e-6)) & 0x3FFF);
    }
    return device_value;
}


const char* DataConverter::getUnitPrefix(double value, double *value_out){
    *value_out = value;
    if(value < 0.0) value = -value;
    if(value >= 1e9){
        *value_out *= 1e-9;
        return "G";
    }
    else if(value >= 1e6){
        *value_out *= 1e-6;
        return "M";
    }
    else if(value >= 1e3){
        *value_out *= 1e-3;
        return "k";
    }
    else if(value >= 1.0){
        return "";
    }
    else if(value >= 1e-3){
        *value_out *= 1e3;
        return "m";
    }
    else if(value >= 1e-6){
        *value_out *= 1e6;
        return "u";
    }

    // fallback: no scaling & no prefix
    return "";
}
