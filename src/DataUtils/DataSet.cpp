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

void DataStream::clear(){
    if(dataType == DataType::DATA_ANALOG){
        for(int i = 0;i < data.doubleData->size();++i){
             (*this->data.doubleData)[i] = 0.0;
        }
    }
    newDataOffset = currentDataOffset = 0.0;
}
void DataStream::add(const DataStream* other){
    if(other->dataType != dataType) return;

    if(dataType == DataType::DATA_ANALOG){
        if(data.doubleData->size() != other->data.doubleData->size())
            return;

        for(int i = 0;i < data.doubleData->size();++i){
             (*this->data.doubleData)[i] += (*other->data.doubleData)[i];
        }
    }
    else if (dataType == DataType::DATA_BITS){
        if(data.bitData->size() != other->data.bitData->size())
            return;

        /* For digital channels, we only use the latest data */
        for(int i = 0;i < data.bitData->size();++i){
             (*this->data.bitData)[i] = (*other->data.bitData)[i];
        }
    }
}

void DataStream::multiply(double value){
    if(dataType == DataType::DATA_ANALOG){
        for(int i = 0;i < data.doubleData->size();++i){
             (*this->data.doubleData)[i] *= value;
        }
    }
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
    this->averaging = 1;
    this->avg_index = 0;
    this->current_y_size = 0;
    this->previous_header[0] = 0;
    this->last_transfer_size = 0;
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

void DataSet::clearYAxises(){
    if(yAxises){
        for(int i = 0;i < current_y_size;++i){
            delete yAxises[i];
        }
        delete[] yAxises;
        yAxises = NULL;
    }
    avg_index = 0;
    avg_count = 0;
}

double DataSet::dataInput(BinaryTransfer* transfer,DataConverter* converter){
    const uint8_t *data = transfer->getData();
    int buffer_count = data[0];
    int header_size = data[0] + 1;
    int y_index = 0;
    /* More buffers than expected */
    if(buffer_count > MAX_BUFFER_COUNT) return 0.0;

    bool buffer_changed = (buffer_count != previous_header[0])
        || (memcmp(data, previous_header, header_size) != 0);

    memcpy(previous_header, data, header_size);

    buffer_changed = buffer_changed || (last_transfer_size != transfer->getSize());
    last_transfer_size = transfer->getSize();

    BufferDescription *buffer_desc = new BufferDescription[buffer_count];
    /* Total number of bytes per sample through all buffers */
    int total_width = 0;
    /* Total length of sample data in bytes */
    int total_length = transfer->getSize() - header_size;
    this->total_channels = 0;
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

    int new_y_size = (averaging > 1)?((averaging + 1) * total_channels):total_channels;

    /* When we disable averaging we need to re-initialize Y buffers*/
    buffer_changed = buffer_changed || ((averaging <= 1) && new_y_size != current_y_size);
        
    if(buffer_changed){
        this->clearYAxises();
        channels = total_channels;
    }
    /* Changing averaging size => reuse some Y buffers */
    else if(new_y_size != current_y_size){
        if(yAxises){
            int old_averaging = (current_y_size / total_channels) - 1;
            if(old_averaging > 1){
                DataStream** newYAxises = new DataStream*[new_y_size];
                memset(newYAxises,0,sizeof(DataStream*)*new_y_size);

                y_index = 0;
                int a_offset = 0;
                if(avg_count > averaging){
                    a_offset = avg_index - averaging;
                    if(a_offset < 0) a_offset += old_averaging;
                }
                else if(avg_count >= old_averaging){
                    a_offset = avg_index;
                }
                for(int i = 0;i < buffer_count;++i){
                    for(int x = 0;x < buffer_desc[i].channels;++x,++y_index){
                        newYAxises[y_index] = yAxises[y_index];

                        /* Copy relevant buffers to new array */
                        for(int a = 0; a < old_averaging;++a){
                            int src = a + a_offset;
                            if(src >= old_averaging) src -= old_averaging;
                            /* Compute absolute index */
                            src = y_index + ((src+1)*total_channels);
                            if(a >= averaging){
                                /* Clear unused buffers */
                                if(yAxises[src]){
                                    delete yAxises[src];
                                }
                            }
                            else {
                                newYAxises[y_index + ((a+1)*total_channels)] = yAxises[src];
                            }
                        }
                    }
                }
                delete[] yAxises;
                yAxises = newYAxises;

                /* compute new avg_count and avg_index */
                avg_count = std::min(avg_count, averaging);
                avg_index = avg_count;
                if(avg_index >= averaging) avg_index -= averaging;
            }
            else {
                this->clearYAxises();
            }
        }
    }
    current_y_size = new_y_size;

    if(yAxises == NULL){
        yAxises = new DataStream*[current_y_size];
        memset(yAxises,0,sizeof(DataStream*)*current_y_size);
    }

    const uint8_t* read_data = data + header_size;
    y_index = 0;
    if(averaging > 1){
        /* When averaging, put data in seperate buffer */
        y_index += (avg_index + 1) * total_channels;
    }
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
        }
        read_data = read_data + buffer_desc[i].size;
    }

    /* Do averaging */
    if(averaging > 1){
        avg_count++;
        if(avg_count > averaging) avg_count = averaging;
        
        /* Initialize average buffer */
        y_index = 0;
        for(int i = 0;i < buffer_count;++i){
            for(int x = 0;x < buffer_desc[i].channels;++x,++y_index){
                DataStream::DataType dataType = (DataStream::DataType)buffer_desc[i].dataType;
                if(yAxises[y_index] && yAxises[y_index]->getType() != dataType){
                    delete yAxises[y_index];
                    yAxises[y_index] = NULL;
                }

                if(yAxises[y_index] == NULL){
                    yAxises[y_index] = new DataStream(dataType, buffer_desc[i].size / buffer_desc[i].totalWidth);
                }
                else {
                    yAxises[y_index]->clear();
                }
            }
        }

        /* Add buffers */
        y_index = 0;
        double mul = 1.0 / avg_count;
        for(int i = 0;i < buffer_count;++i){
            for(int x = 0;x < buffer_desc[i].channels;++x,++y_index){
                if(buffer_desc[i].dataType == DataStream::DATA_ANALOG){
                    for(int a = 0; a < avg_count;++a){
                        yAxises[y_index]->add(yAxises[y_index + (a+1) * total_channels]);
                    }
                    /* Multiply is faster than division in double */
                    yAxises[y_index]->multiply(mul);
                }
                else {
                    /* For digital channels, we only use the latest data */
                    yAxises[y_index]->add(yAxises[y_index + (avg_index + 1) * total_channels]);
                }
            }
        }
        avg_index++;
        if(avg_index >= averaging) avg_index = 0;
    }

    /* Apply offset */
    double inc_offset = 0.0;
    y_index = 0;
    for(int i = 0;i < buffer_count;++i){
        for(int x = 0;x < buffer_desc[i].channels;++x,++y_index){
            if(buffer_desc[i].dataType == DataStream::DATA_BITS){
                if(offsetCalculation == DataSet::OFFSET_NONE){
                    inc_offset += 4.0;
                }
            }
            yAxises[y_index]->applyOffset(inc_offset);
            if(offsetCalculation == DataSet::OFFSET_SPREAD){
                inc_offset += 4.0;
            }
        }
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

    int trig_index = getTriggerChannelIndex(trigger_channel,channel_mask);
    displayOffset = (trig_index < total_channels)?getTriggerOffset(trig_index):0.0;
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
    return (index < total_channels)?yAxises[index]->getOffset():0.0;
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

void DataSet::setAveraging(uint32_t value){
    uint32_t new_value = (value > DATASET_MAX_AVG)?DATASET_MAX_AVG:value;
    if(new_value != this->averaging){
        this->averaging = new_value;
    }
}

void DataSet::resetAverage(){
    this->avg_count = 0;
    this->avg_index = 0;
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
