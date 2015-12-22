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
#ifndef _DATA_AVERAGING_H_
#define _DATA_AVERAGING_H_

#include <QtGui>

template<typename T>
class DataAveraging {
protected:
    QVector<T> accumulator;
    QVector<T> result;
    unsigned count;
    unsigned averageCount;
public:
    DataAveraging(int averageCount){
        this->averageCount = averageCount;
        this->count = 0;
    }

    QVector<T> *getResult(){return &result;}
    void setAveraging(unsigned value){this->averageCount = value;}

    void reset(){count = 0;}
    void commit(const QVector<T> *data){
        if(data->length() != accumulator.length()){
            accumulator.resize(data->length());
            result.resize(data->length());

            accumulator.fill((T)0);
            result.fill((T)0);
            count = 0;
        }

        for(int i = 0;i < data->length();++i){
            accumulator[i] += (*data)[i];
        }

        count++;
        if(count >= averageCount){
            if(count > 0){
                for(int i = 0;i < data->length();++i){
                    result[i] = accumulator[i] / count;
                }
            }
            else {
                for(int i = 0;i < data->length();++i){
                    result[i] = accumulator[i];
                }
            }
            accumulator.fill((T)0);
            count = 0;
        }
    }
};

#endif /* _DATA_AVERAGING_H_ */
