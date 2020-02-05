/*
 * SerialPort.cpp
 *
 *  Created on: Feb 4, 2020
 *      Author: Jonathan
 */

#include <iostream>
#include <stdio.h>
#include <Windows.h>

#include "SerialPort.h"

SerialPort::SerialPort(int baudRate, const std::string &id) {
	Init(baudRate, id);
}

void SerialPort::Init(int baudRate, const std::string &id) {
    // Open the comm device with the overlapped (asynchronous) flag.
    // We only use synchronous reads and writes, but WaitCommEvent
    // requires overlapped mode to accept a timeout and not wait
    // indefinitely. Unfortunately since overlapped mode must be all
    // or nothing, this makes reads and writes more complicated.
    this->handle = CreateFile(
        id.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (this->handle == INVALID_HANDLE_VALUE) {
		perror(("Could not open " + id).c_str());
		throw std::runtime_error("");
    }

    // Modify comm state
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(this->handle, &dcb)) {
		perror("Could not get the comm state");
		throw std::runtime_error("");
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    // Purge any data already present, then apply configuration
    PurgeComm(this->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
    this->SetTimeout(1);
    if (!SetCommState(this->handle, &dcb)) {
		perror("Could not set the comm state");
		throw std::runtime_error("");
    }
}

void SerialPort::SetReadBufferSize(int size) {
	this->readBufferSize = size;
}

void SerialPort::SetTimeout(int timeout) {
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = timeout * 1000;
    if (!SetCommTimeouts(this->handle, &timeouts)) {
		perror("Could not set the timeouts");
		throw std::runtime_error("");
    }
}

void SerialPort::EnableTermination(char c) {
	this->termination = true;
	this->terminationChar = c;
}

void SerialPort::Flush(void) {
    PurgeComm(this->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialPort::Write(const char *data, int length) {
    int spot = 0;
	DWORD n_written = 0;
    OVERLAPPED ov;
	do {
        ov.hEvent = CreateEvent(NULL, true, false, NULL);
        ov.Internal = ov.InternalHigh = 0;
        ov.Offset = ov.OffsetHigh = 0;

        // Write immediately or wait for async completion
        // See Init for why we use an overlapped write
        if (WriteFile(this->handle, &data[spot], length, &n_written, &ov)
            || (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(this->handle, &ov, &n_written, true))) {

            spot += n_written;
        } else {
            perror("Failed to write");
        }

        CloseHandle(ov.hEvent);
	} while (data[spot-1] != terminationChar); 
}

int SerialPort::GetBytesReceived(void) {
    DWORD numErrors;
    COMSTAT stat;
    ClearCommError(this->handle, &numErrors, &stat);
    return stat.cbInQue;
}

int SerialPort::Read(char *data, int size) {
	int spot = 0;
	char buf = '\0';
	memset(data, '\0', size);
    DWORD n_read;
	OVERLAPPED ov;
	do {
        ov.hEvent = CreateEvent(NULL, true, false, NULL);
        ov.Internal = ov.InternalHigh = 0;
        ov.Offset = ov.OffsetHigh = 0;

        // Read immediately or wait for async completion
        // See Init for why we use an overlapped read
        if (ReadFile(this->handle, &buf, 1, &n_read, &ov)
            || (GetLastError() == ERROR_IO_PENDING && GetOverlappedResult(this->handle, &ov, &n_read, TRUE))) {

		    sprintf(&data[spot], "%c", buf);
            spot += n_read;
        } else {
            perror("Failed to read");
        }

        CloseHandle(ov.hEvent);
	} while( buf != terminationChar && spot < size);

	if (spot == 0) {
		std::cout << "Read nothing!" << std::endl;
	}

	return spot;
}

void SerialPort::WaitForData(void)
{
	// Request comm event when characters arrive
	DWORD eventMask = EV_RXCHAR;
	if (!SetCommMask(this->handle, eventMask)) {
		perror("Could not set the comm mask");
		return;
    }

	OVERLAPPED ov;
	ov.hEvent = CreateEvent(NULL, true, false, NULL);
	ov.Internal = ov.InternalHigh = 0;
	ov.Offset = ov.OffsetHigh = 0;
    
    // Wait immediately or wait for async completion
    // This must be an overlapped wait to allow a timeout
	if (WaitCommEvent(this->handle, &eventMask, &ov)
        || (GetLastError() == ERROR_IO_PENDING && WaitForSingleObject(ov.hEvent, 100))) {
		// Wait completed
        // Return value of WaitForSingleObject tells us if it received an event, timed out, or failed
	} else {
		// Wait failed
	}

	SetCommMask(this->handle, 0);
	CloseHandle(ov.hEvent);
}

void SerialPort::Reset(void) {
    PurgeComm(this->handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialPort::Close(void) {
	CloseHandle(this->handle);
}
