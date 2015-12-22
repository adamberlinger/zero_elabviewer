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
#include "DataSet.h"
#include <cmath>

static QVector<uint8_t> *separateBitStream(QVector<uint8_t>* in_result,const uint8_t* data,
        int bytes,int size,int step){

    int out_size = (size * bytes) / step;
    QVector<uint8_t> *result;
    if(in_result){
        in_result->resize(out_size);
        result = in_result;
    }
    else {
        result = new QVector<uint8_t>(out_size);
    }

    for(int i = 0,i_out = 0;i < size;i+=step){
        for(int x = 0; x < bytes;++x,++i_out){
            (*result)[i_out] = data[i+x];
        }
    }
    return result;
}

static int getTriggerChannelIndex(int channel,uint32_t channel_mask){
    if(channel_mask & (0x1 << channel)){
        int i,result = 0;
        for(i = 0;i < channel;++i){
            if(channel_mask & (0x1 << i)){
                result++;
            }
        }
        return result;
    }
    else {
        return 0;
    }
}

DataStream::DataStream(DataStream::DataType dataType, int size){
    this->dataType = dataType;
    if(this->dataType == DataStream::DATA_ANALOG){
        this->data.doubleData = new QVector<double>(size);
    }
    else {
        this->data.bitData = new QVector<uint8_t>(size);
    }
    this->currentDataOffset = this->newDataOffset = 0.0;
}

DataStream::DataStream(QVector<double>* data){
    this->dataType = DATA_ANALOG;
    this->data.doubleData = data;
    this->currentDataOffset = this->newDataOffset = 0.0;
}

DataStream::DataStream(QVector<uint8_t>* data){
    this->dataType = DATA_BITS;
    this->data.bitData = data;
    this->currentDataOffset = this->newDataOffset = 0.0;
}

void DataStream::resize(int size){
    if(this->dataType == DataStream::DATA_ANALOG){
        this->data.doubleData->resize(size);
    }
    else {
        this->data.bitData->resize(size);
    }
}

void DataStream::applyOffset(double value){
    if(this->dataType == DataStream::DATA_ANALOG){
        for(int i = 0; i < this->data.doubleData->length();++i){
            (*this->data.doubleData)[i] += value;
        }
    }
    newDataOffset = currentDataOffset = value;
}

void DataStream::removeOffset(){
    if(this->dataType == DataStream::DATA_ANALOG){
        for(int i = 0; i < this->data.doubleData->length();++i){
            (*this->data.doubleData)[i] -= currentDataOffset;
        }
    }
    newDataOffset = currentDataOffset = 0.0;
}

DataStream::~DataStream(){
    if(this->dataType == DataStream::DATA_ANALOG){
        delete this->data.doubleData;
    }
    else {
        delete this->data.bitData;
    }
}

DataSet::DataSet(int channels, double fs){
    if(channels < 1){
        channels = 1;
    }

    this->channel_mask = (0x1 << channels) - 1;
    this->trigger_channel = 0;

    this->approx = APPROX_SHIFT;

    this->channels = channels;
    this->xAxis = NULL;
    this->yAxises = NULL;
    this->size = 0;
    this->fs = fs;
    this->displayOffset = 0.0;
    this->offsetCalculation = DataSet::OFFSET_SPREAD;
}

double DataSet::setOffsetCalculation(DataSet::OffsetCalculation type, double value){
    double inc_offset = 0.0;
    if(this->offsetCalculation != type){
        if(type == DataSet::OFFSET_SPREAD){
            for(int i = 0;i < channels;++i){
                yAxises[i]->applyOffset(inc_offset);
                inc_offset+=4.0;
            }
        }
        else {
            for(int i = 0;i < channels;++i){
                yAxises[i]->removeOffset();
                if(yAxises[i]->getType() == DataStream::DATA_BITS){
                    if(i!=0)inc_offset+=4.0;
                    yAxises[i]->applyOffset(inc_offset);
                }
            }
            inc_offset+=4.0;
        }
    }
    this->offsetCalculation = type;
    return inc_offset;
}

double DataSet::dataInput(BinaryTransfer* transfer,DataConverter* converter){
    const uint8_t *data = transfer->getData();
    int buffer_count = data[0];
    int header_size = data[0] + 1;
    BufferDescription *buffer_desc = new BufferDescription[buffer_count];
    /* Total number of bytes per sample through all buffers */
    int total_width = 0;
    /* Total length of sample data in bytes */
    int total_length = transfer->getSize() - header_size;
    int total_channels = 0;
    for(int i = 0;i < buffer_count;++i){
        buffer_desc[i].init(data[i+1]);
        total_width += buffer_desc[i].totalWidth;
        total_channels += buffer_desc[i].channels;
    }
    for(int i = 0;i < buffer_count;++i){
        buffer_desc[i].size = (total_length * buffer_desc[i].totalWidth) / total_width;
    	if((buffer_desc[i].size * total_width) != (total_length * buffer_desc[i].totalWidth)){
    	  /* Data not aligned */
    	  /* TODO: report error */
    	  return 3.0;
    	}
    }
    int sample_count = buffer_desc[0].size / (buffer_desc[0].totalWidth);

    if(channels != total_channels){
        if(yAxises){
            for(int i = 0;i < channels;++i){
                delete yAxises[i];
            }
            delete[] yAxises;
            yAxises = NULL;
        }
        channels = total_channels;
    }

    if(yAxises == NULL){
        yAxises = new DataStream*[total_channels];
        memset(yAxises,0,sizeof(DataStream*)*total_channels);
    }

    const uint8_t* read_data = data + header_size;
    int y_index = 0;
    double inc_offset = 0.0;
    for(int i = 0;i < buffer_count;++i){
        for(int x = 0;x < buffer_desc[i].channels;++x,++y_index){
            if(yAxises[y_index] && yAxises[y_index]->getType() != buffer_desc[i].dataType){
                delete yAxises[y_index];
                yAxises[y_index] = NULL;
            }
            if(buffer_desc[i].dataType == DataStream::DATA_ANALOG){
                /* TODO: fix sample width */
                if(yAxises[y_index]){
                    converter->fromDevice16BitStream(yAxises[y_index]->getDouble(),
                        read_data+x*buffer_desc[i].sampleWidth,buffer_desc[i].size,
                        buffer_desc[i].totalWidth);
                }
                else {
                    yAxises[y_index] = new DataStream(converter->fromDevice16BitStream(NULL,
                        read_data+x*buffer_desc[i].sampleWidth,buffer_desc[i].size,
                        buffer_desc[i].totalWidth));
                }
            }
            else if(buffer_desc[i].dataType == DataStream::DATA_BITS){
                if(offsetCalculation == DataSet::OFFSET_NONE){
                    inc_offset += 4.0;
                }

                if(yAxises[y_index]){
                    separateBitStream(yAxises[y_index]->getBits(),
                        read_data+x*buffer_desc[i].sampleWidth, buffer_desc[i].sampleWidth,
                        buffer_desc[i].size, buffer_desc[i].totalWidth);
                }
                else {
                    yAxises[y_index] = new DataStream(separateBitStream(NULL,
                        read_data+x*buffer_desc[i].sampleWidth, buffer_desc[i].sampleWidth,
                        buffer_desc[i].size, buffer_desc[i].totalWidth));
                }
            }
            yAxises[y_index]->applyOffset(inc_offset);
            if(offsetCalculation == DataSet::OFFSET_SPREAD){
                inc_offset += 4.0;
            }
        }
        read_data = read_data + buffer_desc[i].size;
    }

    if(offsetCalculation == DataSet::OFFSET_NONE){
        inc_offset += 4.0;
    }

    if(size != sample_count && xAxis != NULL){
        xAxis->resize(sample_count);
    }
    size = sample_count;
    if(xAxis == NULL){
        xAxis = new QVector<double>(size);
    }

    displayOffset = getTriggerOffset(getTriggerChannelIndex(trigger_channel,channel_mask));
    if(approx == DataSet::APPROX_SHIFT){
        for(int i = 0;i < size;++i){
            (*xAxis)[i] = (i + displayOffset) / fs;
        }
    }
    else if(approx == DataSet::APPROX_LINEAR){
        /* TODO: implement linear shift */
    }
    return inc_offset;
}

void DataSet::setChannelMask(uint32_t value){
    this->channel_mask = value;
}

void DataSet::setTriggerChannel(int value){
    this->trigger_channel = value;
}

void DataSet::setOffset(int channel, double value){
    if(channel >= 0 && channel < this->channels){
        yAxises[channel]->setOffset(value);
    }
}

double DataSet::getTriggerChannelOffset(){
    int index = getTriggerChannelIndex(trigger_channel,channel_mask);
    return yAxises[index]->getOffset();
}

double DataSet::getTriggerOffset(int index){
    int i = std::floor(size * trigger_x) - 1;
    if(i < 0) i = 0;
    if((i+1) >= size){
        return 0.0f;
    }
    float y1 = (float)(*yAxises[index]->getDouble())[i] - yAxises[index]->getOffset();
    float y2 = (float)(*yAxises[index]->getDouble())[i+1] - yAxises[index]->getOffset();
    float ty = trigger_y;
    bool triggerValid = false;
    triggerValid |= ((ty >= y1) && (ty <= y2));
    triggerValid |= ((ty >= y2) && (ty <= y1));
    if(triggerValid)
        return 1.0f - (ty - y1) / (y2 - y1);
    else {
        return 0.0f;
    }
}

DataSet::~DataSet(){
    if(xAxis){
        delete xAxis;
        for(int i = 0; i < this->channels;++i){
            delete yAxises[i];
        }
        delete[] yAxises;
    }
}
