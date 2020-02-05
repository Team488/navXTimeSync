/*
 * SerialPort.h
 *
 *  Created on: Feb 4, 2020
 *      Author: Jonathan
 */

#pragma once

#include <string>

class ISerialPort {
public:
    ISerialPort(void) {}
    virtual void Init(int baudRate, const std::string &id) = 0;
    virtual void SetReadBufferSize(int size) = 0;
    virtual void SetTimeout(int timeout) = 0;
    virtual void EnableTermination(char c) = 0;
    virtual void Flush(void) = 0;
    virtual void Write(const char *data, int length) = 0;
    virtual int  GetBytesReceived(void) = 0;
    virtual int  Read(char *data, int size) = 0;
    virtual void WaitForData(void) = 0;
    virtual void Reset(void) = 0;
    virtual void Close(void) = 0;
};
