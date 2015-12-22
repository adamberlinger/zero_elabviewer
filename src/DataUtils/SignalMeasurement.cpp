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
#include "SignalMeasurement.h"

SignalMeasurement::SignalMeasurement(){
    average = 0.0f;
}

void SignalMeasurement::input(const QVector<double>* data){
    average = 0.0;
    minValue = (*data)[0];
    maxValue = (*data)[0];

    for(int i = 0;i < data->length();++i){
        double value = (*data)[i];
        average += value;
        if(minValue > value){
            minValue = value;
        }
        if(maxValue < value){
            maxValue = value;
        }
    }

    average /= data->length();

    noise = 0.0;
    for(int i = 0;i < data->length();++i){
        double x = average - (*data)[i];
        noise += x*x;
    }

    noise /= data->length();

    noise = sqrt(noise);
}

SignalMeasurement::~SignalMeasurement(){

}
