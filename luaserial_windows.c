#include "luamod.h"

#include "luaserial_base.c"

#include <windows.h>
#include <io.h>

static char * last_error(void) {
	DWORD err = 0;
	DWORD ret = 0;
	static char errbuf[MAX_PATH+1] = {0};
	static char retbuf[MAX_PATH+1] = {0};
	err = GetLastError();
	ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, errbuf, MAX_PATH, NULL);
	if (ret >= 2) {
		/* discard CRLF characters */
		errbuf[ret-2] = 0;
	} else {
		strcpy(errbuf, "n/a");
	}
	snprintf(retbuf, MAX_PATH, "\"%s\" (%lu)", errbuf, err);
	return retbuf;
}

/*static int open(lua_State *l) {
	const char *name;
	DWORD share;
  COMMTIMEOUTS commTimeouts;
	int fd;
	HANDLE hFile;
	name = luaL_checkstring(L, 1);
	share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	//share = 0; // exclusive access
	hFile = CreateFile(name, GENERIC_READ | GENERIC_WRITE, share, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	memset(&commTimeouts, 0, sizeof(COMMTIMEOUTS));
  commTimeouts.ReadIntervalTimeout = 0;
  commTimeouts.ReadTotalTimeoutMultiplier = 0;
  commTimeouts.ReadTotalTimeoutConstant = 0;
  commTimeouts.WriteTotalTimeoutConstant = 0;
  commTimeouts.WriteTotalTimeoutMultiplier = 0;
  if (!SetCommTimeouts(hFile, &commTimeouts)) {
    CloseHandle(hFile);
		RETURN_ERROR(l, last_error())
  }
  PurgeComm(hFile, PURGE_RXCLEAR);
  PurgeComm(hFile, PURGE_TXCLEAR);
	fd = _open_osfhandle(hFile, _O_RDWR);
  if (fd < 0) {
    CloseHandle(hFile);
		RETURN_ERROR(l, last_error())
  }
	lua_pushinteger(l, fd);
	return 1;
}*/

/*static int reopen(lua_State *l) {
	int fd;
	HANDLE hFile;
	HANDLE hReFile;
	DWORD access;
	DWORD share;
	DWORD flags;
	fd = getFileDesc(l, 1);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
	share = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
	flags = FILE_FLAG_OVERLAPPED;
	hReFile = ReOpenFile(hFile, access, share, flags);
	if (hReFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	fd = _open_osfhandle(hReFile, _O_RDWR);
  if (fd < 0) {
    CloseHandle(hReFile);
		RETURN_ERROR(l, last_error())
  }
	lua_pushinteger(l, fd);
	return 1;
}*/

/*
 * Calling the CancelIoEx function does not guarantee that an I/O operation will be canceled;
 * the driver which is handling the operation must support cancellation and the operation must be in a state that can be canceled.
 * see https://msdn.microsoft.com/en-us/library/windows/desktop/aa363789(v=vs.85).aspx
 * 
 * we may also use CancelSynchronousIo(hThread)
 */
// not available in mingw
/*static int cancelIo(lua_State *l) {
	int fd;
	HANDLE hFile;
	fd = getFileDesc(l, 1);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	if (!CancelIoEx(hFile, NULL)) {
		if (GetLastError() != ERROR_NOT_FOUND) {
			RETURN_ERROR(l, last_error())
		}
	}
	// the read or write operation will fail with ERROR_OPERATION_ABORTED
	RETURN_SUCCESS(l)
}*/

static int getSerial(lua_State *l) {
	int fd;
	DCB pdcb;
	HANDLE hFile;
	int stopBits;
	int parity;
	fd = getFileDesc(l, 1);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	memset(&pdcb, 0, sizeof(DCB));
	pdcb.DCBlength = sizeof(DCB);
	if (!GetCommState(hFile, &pdcb)) {
		RETURN_ERROR(l, last_error())
	}
	switch(pdcb.StopBits) {
	case ONESTOPBIT:
		stopBits = 1;
		break;
	case TWOSTOPBITS:
		stopBits = 2;
		break;
	case ONE5STOPBITS:
		stopBits = 3;
		break;
	}
	switch(pdcb.Parity) {
	case NOPARITY:
		parity = 0;
		break;
	case ODDPARITY:
		parity = 1;
		break;
	case EVENPARITY:
		parity = 2;
		break;
	case MARKPARITY:
		parity = 3;
		break;
	case SPACEPARITY:
		parity = 4;
		break;
	}
	/*
	lua_pushinteger(l, pdcb.BaudRate);
	lua_pushinteger(l, pdcb.ByteSize);
	lua_pushinteger(l, stopBits);
	lua_pushinteger(l, parity);
	return 4;
	*/
	lua_newtable(l);
	SET_TABLE_KEY_INTEGER(l, "baudRate", pdcb.BaudRate)
	SET_TABLE_KEY_INTEGER(l, "dataBits", pdcb.ByteSize)
	SET_TABLE_KEY_INTEGER(l, "stopBits", stopBits)
	SET_TABLE_KEY_INTEGER(l, "parity", parity)
	return 1;
}

static void setBaudRate(LPDCB lpDCB, int baudRate) {
	switch(baudRate) {
	case 110:
		lpDCB->BaudRate = CBR_110;
		break;
	case 300:
		lpDCB->BaudRate = CBR_300;
		break;
	case 600:
		lpDCB->BaudRate = CBR_600;
		break;
	case 1200:
		lpDCB->BaudRate = CBR_1200;
		break;
	case 2400:
		lpDCB->BaudRate = CBR_2400;
		break;
	case 4800:
		lpDCB->BaudRate = CBR_4800;
		break;
	case 9600:
		lpDCB->BaudRate = CBR_9600;
		break;
	case 14400:
		lpDCB->BaudRate = CBR_14400;
		break;
	case 19200:
		lpDCB->BaudRate = CBR_19200;
		break;
	case 38400:
		lpDCB->BaudRate = CBR_38400;
		break;
	case 57600:
		lpDCB->BaudRate = CBR_57600;
		break;
	case 115200:
		lpDCB->BaudRate = CBR_115200;
		break;
	case 128000:
		lpDCB->BaudRate = CBR_128000;
		break;
	case 256000:
		lpDCB->BaudRate = CBR_256000;
		break;
	}
}

static int getIntegerField(lua_State *l, int i, const char *k, int def) {
	int v;
	//lua_pushstring(l, "baud_rate"); lua_gettable(l, 2);
	lua_getfield(l, i, k);
	if (lua_isinteger(l, -1)) {
		v = (int) lua_tointeger(l, -1);
	} else {
		v = def;
	}
	lua_pop(l, 1);
	return v;
}

/*
 * Configures a communications device according to the specifications in a device-control block (a DCB structure).
 * The function reinitializes all hardware and control settings, but it does not empty output or input queues.
 * see https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa363436(v=vs.85).aspx
 * see also BuildCommDCB https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa363143(v=vs.85).aspx
 */
static int setSerial(lua_State *l) {
	int baudRate;
	int dataBits;
	int stopBits;
	int parity;
	int fd;
	DCB pdcb;
	HANDLE hFile;
  trace("setSerial()\n");
	fd = getFileDesc(l, 1);
	if (lua_istable(l, 2)) {
		baudRate = getIntegerField(l, 2, "baudRate", -1);
		dataBits = getIntegerField(l, 2, "dataBits", -1);
		stopBits = getIntegerField(l, 2, "stopBits", -1);
		parity = getIntegerField(l, 2, "parity", -1);
	} else {
		baudRate = (int) luaL_optinteger(l, 2, -1);
		dataBits = (int) luaL_optinteger(l, 3, -1);
		stopBits = (int) luaL_optinteger(l, 4, -1);
		parity = (int) luaL_optinteger(l, 5, -1);
	}
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	memset(&pdcb, 0, sizeof(DCB));
	pdcb.DCBlength = sizeof(DCB);
	// retrieve current state in order to keep non modified fields
	if (!GetCommState(hFile, &pdcb)) {
		RETURN_ERROR(l, last_error())
	}
	if (baudRate >= 0) {
		//pdcb.BaudRate = baudRate;
		setBaudRate(&pdcb, baudRate);
	}
	if (dataBits >= 0) {
		pdcb.ByteSize = dataBits;
	}
	if (parity >= 0) {
		// we probably need to set the field fParity accordingly
		switch(parity) {
		case 0:
			pdcb.Parity = NOPARITY;
			break;
		case 1:
			pdcb.Parity = ODDPARITY;
			break;
		case 2:
			pdcb.Parity = EVENPARITY;
			break;
		case 3:
			pdcb.Parity = MARKPARITY;
			break;
		case 4:
			pdcb.Parity = SPACEPARITY;
			break;
		}
	}
	if (stopBits >= 0) {
		switch(stopBits) {
		case 1:
			pdcb.StopBits = ONESTOPBIT;
			break;
		case 2:
			pdcb.StopBits = TWOSTOPBITS;
			break;
		case 3:
			pdcb.StopBits = ONE5STOPBITS;
			break;
		}
	}
	if (!SetCommState(hFile, &pdcb)) {
		RETURN_ERROR(l, last_error())
	}
	RETURN_SUCCESS(l)
}

static int setTimeout(lua_State *l) {
	int fd;
	int readTimeOut;
	int writeTimeOut;
	HANDLE hFile;
	COMMTIMEOUTS t;
	fd = getFileDesc(l, 1);
	readTimeOut = luaL_checkinteger(l, 2);
	writeTimeOut = luaL_checkinteger(l, 3);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	memset(&t, 0, sizeof(COMMTIMEOUTS));
	/*if (!GetCommTimeouts(hFile, &t)) {
		RETURN_ERROR(l, last_error())
	}*/
	if (readTimeOut < 0) {
		// no timeout
		t.ReadIntervalTimeout = 0;
		t.ReadTotalTimeoutMultiplier = 0;
		t.ReadTotalTimeoutConstant = 0;
	} else if (readTimeOut == 0) {
		// read immediate
		t.ReadIntervalTimeout = MAXDWORD;
		t.ReadTotalTimeoutMultiplier = 0;
		t.ReadTotalTimeoutConstant = 0;
	} else {
		t.ReadIntervalTimeout = 0;
		t.ReadTotalTimeoutMultiplier = 0;
		t.ReadTotalTimeoutConstant = readTimeOut;
	}
	if (writeTimeOut < 0) {
		// no timeout
		writeTimeOut = 0;
	}
	t.WriteTotalTimeoutMultiplier = 0;
	t.WriteTotalTimeoutConstant = writeTimeOut;
	if (!SetCommTimeouts(hFile, &t)) {
		RETURN_ERROR(l, last_error())
	}
	RETURN_SUCCESS(l)
}

/*
 * The WaitCommEvent function should be executed as an overlapped operation
 * so the other threads of the process can perform I/O operations during the wait.
 * see https://msdn.microsoft.com/fr-fr/library/windows/desktop/aa363424(v=vs.85).aspx
 */
/*static int waitDataAvailable(lua_State *l) {
	int fd;
	//int waitTimeOut;
	DWORD dwEventMask;
	HANDLE hFile;
	fd = getFileDesc(l, 1);
	//waitTimeOut = luaL_checkinteger(l, 2);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	if (!SetCommMask(hFile, EV_RXCHAR)) {
		RETURN_ERROR(l, last_error())
	}
	if (!WaitCommEvent(hFile, &dwEventMask, NULL)) {
		RETURN_ERROR(l, last_error())
	}
	RETURN_SUCCESS(l)
}*/

static int flush(lua_State *l) {
	int fd;
	fd = getFileDesc(l, 1);
	if (!FlushFileBuffers(fd)) {
		RETURN_ERROR(l, last_error())
	}
	if (!PurgeComm(fd, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR)) {
		RETURN_ERROR(l, last_error())
	}
	RETURN_SUCCESS(l)
}

static int waitDataAvailable(lua_State *l) {
	int fd;
	int waitTimeOut;
	int sleepTime;
	int sleepTotalTime;
	unsigned long errmask = 0;
	COMSTAT cs;
	HANDLE hFile;
	fd = getFileDesc(l, 1);
	waitTimeOut = luaL_checkinteger(l, 2);
	sleepTime = luaL_optinteger(l, 3, 500);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (hFile == INVALID_HANDLE_VALUE) {
		RETURN_ERROR(l, last_error())
	}
	sleepTotalTime = 0;
	while ((waitTimeOut < 0) || (sleepTotalTime < waitTimeOut)) {
		if (!ClearCommError(hFile, &errmask, &cs)) {
			RETURN_ERROR(l, last_error())
		}
		if (cs.cbInQue > 0) {
			RETURN_SUCCESS(l)
		}
		Sleep(sleepTime);
		sleepTotalTime += sleepTime;
	}
	RETURN_ERROR(l, "timeout")
}

static int available(lua_State *l) {
	int fd;
	HANDLE hFile;
	unsigned long errmask = 0;
	COMSTAT cs;
	fd = getFileDesc(l, 1);
	hFile = (HANDLE)_get_osfhandle(fd);
	if (!ClearCommError(hFile, &errmask, &cs)) {
		RETURN_ERROR(l, last_error())
	}
	lua_pushinteger(l, cs.cbInQue);
	return 1;
}
