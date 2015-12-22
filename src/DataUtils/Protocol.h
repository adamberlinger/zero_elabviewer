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
#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdint.h>
#include <QtGui>

class BinaryTransfer{
protected:
    int channel;
    uint8_t* data;
    int size;
    int written;
public:
    BinaryTransfer(int channel);
    void allocate(int size);
    void write(const uint8_t* data, int length);
    int remainingSize();
    int getSize()const{return size;}
    const uint8_t* getData()const{return data;}
    int getChannel()const{return channel;}
    virtual ~BinaryTransfer();
};

class Command{
public:
    uint8_t channel;
    uint8_t command_id;
    uint32_t value;
};

class DeviceDescription : public QObject {
    Q_OBJECT
public:
    enum Capabilities {
        OSCILLOSCOPE = 1,
        PWM = 2,
        VOLTMETER = 3,
        PWM_IN = 4,
        GENERATOR = 5
    };

    QString name;
    QString configurationName;
    uint32_t capabilities;

    DeviceDescription();
    virtual ~DeviceDescription();
    bool hasCapability(Capabilities cap);
    void clear();
signals:
    void valuesChanged();
};

class Protocol : public QObject{
    Q_OBJECT
protected:
    enum ProtocolState {
        IDLE,
        /* Binary transfer */
        BINARY_START, BINARY_LENGTH_1, BINARY_LENGTH_2, BINARY_DATA,
        /* Command transfer */
        COMMAND_VALUE, COMMAND_CHANNEL, COMMAND_ID,
    };

    struct CoreProtocolState {
        uint8_t last_command;
        bool last_command_valid;
    };

    uint8_t* buffer;
    int bufferSize;
    QIODevice *device;

    ProtocolState state;
    CoreProtocolState coreState;
    DeviceDescription deviceDescription;
    BinaryTransfer* currentTransfer;
    Command currentCommand;
    int binarySize;

    void processChunk(int size);
    void coreTransfer(BinaryTransfer* transfer);
signals:
    void binaryReceived();
    void commandReceived();
    void deviceReconnected();
    void supplyVoltageValue(double);
public slots:
    void nextDeviceConfiguration();
public:
    Protocol(int bufferSize);
    void setDevice(QIODevice* device);
    void processData();
    DeviceDescription* getDeviceDescription(){return &deviceDescription;}
    void command(uint8_t cmd,int channel,uint32_t value);
    bool isConnected(){return device != NULL;}
    BinaryTransfer* popTransfer(int channel);
    const Command* getLastCommand(){return &currentCommand; }
};

#endif /* _PROTOCOL_H_ */
