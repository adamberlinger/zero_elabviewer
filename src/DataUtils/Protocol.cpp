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
#include "Protocol.h"
#include <iostream>

BinaryTransfer::BinaryTransfer(int channel){
    this->channel = channel;
    this->data = NULL;
    this->size = 0;
    this->written = 0;
}

void BinaryTransfer::allocate(int size){
    this->data = new uint8_t[size];
    if(data){
        this->size = size;
        this->written = 0;
    }
}

void BinaryTransfer::write(const uint8_t* data, int length){
    int toWrite = this->remainingSize();
    if(toWrite > length) toWrite = length;
    memcpy(this->data+written,data,toWrite);
    written+=toWrite;
}

int BinaryTransfer::remainingSize(){
    return size - written;
}

BinaryTransfer::~BinaryTransfer(){
    if(data){
        delete[] data;
    }
}

DeviceDescription::DeviceDescription(){
    capabilities = 0xFFFFFFFF;
}


bool DeviceDescription::hasCapability(DeviceDescription::Capabilities cap){
    return (this->capabilities & (1 << cap)) > 0;
}

void DeviceDescription::clear(){
    name = "";
    configurationName = "";
}

DeviceDescription::~DeviceDescription(){

}

Protocol::Protocol(int bufferSize):bufferSize(bufferSize){
    this->device = NULL;
    this->buffer =  new uint8_t[bufferSize];
    this->coreState.last_command_valid = false;
}

void Protocol::setDevice(QIODevice* device){
    this->device = device;
    this->state = Protocol::IDLE;
    this->deviceDescription.capabilities = 0xFFFFFFFF;
    deviceDescription.valuesChanged();
    this->deviceReconnected();

    coreState.last_command_valid = true;
    coreState.last_command = 'N';
    this->command('G',0,'N');
}

void Protocol::nextDeviceConfiguration(){
    this->command('C',0,0);

    this->deviceReconnected();
    coreState.last_command_valid = true;
    coreState.last_command = 'N';
    this->command('G',0,'N');
}

void Protocol::processData(){
    if(device){
        int bytes = device->bytesAvailable();
        while(bytes > 0){
            int x = device->read((char*)buffer,bufferSize);
            this->processChunk(x);
            bytes -= x;
            if(x <= 0)
                break;
        }
    }
}

void Protocol::command(uint8_t cmd,int channel,uint32_t value){
    char cmd_buffer[7];
    cmd_buffer[0] = 0xFE;
    cmd_buffer[1] = (uint8_t)(value & 0xFF);
    cmd_buffer[2] = (uint8_t)((value >> 8) & 0xFF);
    cmd_buffer[3] = (uint8_t)((value >> 16) & 0xFF);
    cmd_buffer[4] = (uint8_t)((value >> 24) & 0xFF);
    cmd_buffer[5] = cmd;
    cmd_buffer[6] = (uint8_t)channel;
    int i = 0;
    if(device){
        do{
            i += device->write(cmd_buffer+i,7-i);
        }while(i < 7);
    }
}

BinaryTransfer* Protocol::popTransfer(int channel){
    if(currentTransfer && currentTransfer->getChannel() == channel){
        return currentTransfer;
    }
    return NULL;
}

void Protocol::processChunk(int size){
    for(int i = 0;i < size;++i){
        switch(state){
            case Protocol::IDLE:
                if(buffer[i] == 0xFF){
                    state = Protocol::BINARY_START;
                }
                else if(buffer[i] == 0xFE){
                    state = Protocol::COMMAND_VALUE;
                    binarySize = 0;
                    currentCommand.value = 0;
                }
                else {
                    std::cout << buffer[i];
                    if(buffer[i] == '\n'){
                        std::cout << std::flush;
                    }
                }
                break;
            case Protocol::COMMAND_VALUE:
                currentCommand.value |= ((uint32_t)buffer[i] << (8*binarySize));
                binarySize++;
                if(binarySize == 4){
                    state = Protocol::COMMAND_ID;
                }
                break;
            case Protocol::COMMAND_ID:
                currentCommand.command_id = buffer[i];
                state = Protocol::COMMAND_CHANNEL;
                break;
            case Protocol::COMMAND_CHANNEL:
                currentCommand.channel = buffer[i];
                state = Protocol::IDLE;
                this->commandReceived();
                break;

            case Protocol::BINARY_START:
                currentTransfer = new BinaryTransfer((int)buffer[i]);
                //std::cout << "Receiving binary data" << std::endl;
                state = BINARY_LENGTH_1;
                break;
            case Protocol::BINARY_LENGTH_1:
                binarySize = (int)buffer[i];
                state = BINARY_LENGTH_2;
                break;
            case Protocol::BINARY_LENGTH_2:
                binarySize += ((int)buffer[i]) << 8;
                currentTransfer->allocate(binarySize);
                state = BINARY_DATA;
                break;
            case Protocol::BINARY_DATA:{
                    int length = currentTransfer->remainingSize();
                    if(length > (size - i)){
                        length = size - i;
                        currentTransfer->write(buffer+i,length);
                    }
                    else {
                        currentTransfer->write(buffer+i,length);
                        if(currentTransfer->getChannel() == 0){
                            this->coreTransfer(currentTransfer);
                        }
                        else {
                            this->binaryReceived();
                        }
                        //std::cout << "Binary transfer of size " << currentTransfer->getSize() << " received" << std::endl;
                        state = Protocol::IDLE;
                    }
                    i += (length-1);
                }
                break;
            default:
                break;
        }
    }
}

void Protocol::coreTransfer(BinaryTransfer* transfer){
    const uint8_t* data = transfer->getData();
    if(coreState.last_command_valid){
        if(coreState.last_command == 'N'){
            if(data[transfer->getSize()-1] == '\0'){
                deviceDescription.name = QString((const char*)data);
                qDebug() << "Target name: " << deviceDescription.name;
                deviceDescription.valuesChanged();
            }
            this->command('G',0,'V');
            coreState.last_command = 'V';
            coreState.last_command_valid = true;
        }
        else if(coreState.last_command == 'V'){
            if(transfer->getSize() == 4){
                uint32_t src_value = *(uint32_t*)transfer->getData();
                qDebug() << "Src voltage: " << src_value << "V";
                double supplyVoltage = src_value * 0.001;
                qDebug() << "Supply voltage: " << supplyVoltage << "V";
                supplyVoltageValue(supplyVoltage);
                coreState.last_command_valid = false;
            }
            this->command('G',0,'C');
            coreState.last_command = 'C';
            coreState.last_command_valid = true;
        }
        else if(coreState.last_command == 'C'){
            if(data[transfer->getSize()-1] == '\0'){
                deviceDescription.configurationName = QString((const char*)data);
                qDebug() << "Configuration name: " << deviceDescription.configurationName;
                deviceDescription.valuesChanged();
                coreState.last_command_valid = false;
            }
            this->command('G',0,'F');
            coreState.last_command = 'F';
            coreState.last_command_valid = true;
        }
        else if(coreState.last_command == 'F'){
            if(transfer->getSize() == 4){
                uint32_t src_value = *(uint32_t*)transfer->getData();
                deviceDescription.capabilities = src_value;
                coreState.last_command_valid = false;
                deviceDescription.valuesChanged();
            }
        }
    }
}
