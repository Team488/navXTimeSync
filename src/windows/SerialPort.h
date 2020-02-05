/*
 * SerialPort.h
 *
 *  Created on: Feb 4, 2020
 *      Author: Jonathan
 */

#pragma once

#include <string>
#include <Windows.h>

#include "../ISerialPort.h"

class SerialPort : public ISerialPort {

	private:
		HANDLE handle;
        int readBufferSize;
        bool termination;
        char terminationChar;

    public:
		SerialPort(int baudRate, const std::string &id);
		void Init(int baudRate, const std::string &id);
		void SetReadBufferSize(int size);
		void SetTimeout(int timeout);
		void EnableTermination(char c);
		void Flush(void);
		void Write(const char *data, int length);
		int GetBytesReceived(void);
		int Read(char *data, int size);
		void WaitForData(void);
		void Reset(void);
		void Close(void);
};
