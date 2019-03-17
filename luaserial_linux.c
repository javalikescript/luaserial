#include "luamod.h"

#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "luaserial_base.c"

static int getSerial(lua_State *l) {
	int fd;
	int baudRate;
	int dataBits;
	int stopBits;
	int parity;
	struct termios term;
	fd = getFileDesc(l, 1);
	if (tcgetattr(fd, &term) < 0) {
		RETURN_ERROR(l, strerror(errno))
	}
	speed_t speed = cfgetispeed(&term);
	switch(speed) {
	case B0:
		baudRate = 0;
		break;
	case B110:
		baudRate = 110;
		break;
	case B300:
		baudRate = 300;
		break;
	case B600:
		baudRate = 600;
		break;
	case B1200:
		baudRate = 1200;
		break;
	case B1800:
		baudRate = 1800;
		break;
	case B2400:
		baudRate = 2400;
		break;
	case B9600:
		baudRate = 9600;
		break;
	case B19200:
		baudRate = 19200;
		break;
	case B38400:
		baudRate = 38400;
		break;
	case B57600:
		baudRate = 57600;
		break;
	case B115200:
		baudRate = 115200;
		break;
	case B230400:
		baudRate = 230400;
		break;
	default:
		baudRate = -1;
		break;
	}
	lua_pushinteger(l, baudRate);
	return 1;
}

static void setBaudRate(struct termios *pterm, int baudRate) {
	switch(baudRate) {
	case 0:
		cfsetispeed(pterm, B0);
		cfsetospeed(pterm, B0);
		break;
	case 110:
		cfsetispeed(pterm, B110);
		cfsetospeed(pterm, B110);
		break;
	case 300:
		cfsetispeed(pterm, B300);
		cfsetospeed(pterm, B300);
		break;
	case 600:
		cfsetispeed(pterm, B600);
		cfsetospeed(pterm, B600);
		break;
	case 1200:
		cfsetispeed(pterm, B1200);
		cfsetospeed(pterm, B1200);
		break;
	case 1800:
		cfsetispeed(pterm, B1800);
		cfsetospeed(pterm, B1800);
		break;
	case 2400:
		cfsetispeed(pterm, B2400);
		cfsetospeed(pterm, B2400);
		break;
	case 4800:
		cfsetispeed(pterm, B4800);
		cfsetospeed(pterm, B4800);
		break;
	case 9600:
		cfsetispeed(pterm, B9600);
		cfsetospeed(pterm, B9600);
		break;
	case 19200:
		cfsetispeed(pterm, B19200);
		cfsetospeed(pterm, B19200);
		break;
	case 38400:
		cfsetispeed(pterm, B38400);
		cfsetospeed(pterm, B38400);
		break;
	case 57600:
		cfsetispeed(pterm, B57600);
		cfsetospeed(pterm, B57600);
		break;
	case 115200:
		cfsetispeed(pterm, B115200);
		cfsetospeed(pterm, B115200);
		break;
	case 230400:
		cfsetispeed(pterm, B230400);
		cfsetospeed(pterm, B230400);
		break;
	}
}

static int setSerial(lua_State *l) {
	int fd;
	int baudRate;
	int dataBits;
	int stopBits;
	int parity;
	int init;
	struct termios term;
	trace("setSerial()\n");
	fd = getFileDesc(l, 1);
	baudRate = (int) luaL_optinteger(l, 2, -1);
	dataBits = (int) luaL_optinteger(l, 3, -1);
	stopBits = (int) luaL_optinteger(l, 4, -1);
	parity = (int) luaL_optinteger(l, 5, -1);
	init = (!lua_isboolean(l, 6)) || lua_toboolean(l, 6);
	if (tcgetattr(fd, &term) < 0) {
		RETURN_ERROR(l, strerror(errno))
	}
	if (init) {
		//bzero(&term, sizeof(term));
		// see http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html
		// control mode flags
		// term.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
		term.c_cflag |= (CREAD | CLOCAL); // CREAD: enable receiving characters; CLOCAL: local connection, no modem contol
		// input mode flags
		term.c_iflag = IGNPAR; // IGNPAR: ignore bytes with parity errors; ICRNL: map CR to NL
		// output mode flags
		term.c_oflag = 0;
		// local mode flags
		term.c_lflag = ICANON; // ICANON: enable canonical input
		// In canonical mode: Input is made available line by line.
		// control characters
		term.c_cc[VINTR] = _POSIX_VDISABLE;
		term.c_cc[VQUIT] = _POSIX_VDISABLE;
		term.c_cc[VSTART] = _POSIX_VDISABLE;
		term.c_cc[VSTOP] = _POSIX_VDISABLE;
		term.c_cc[VSUSP] = _POSIX_VDISABLE;
		term.c_cc[VEOF] = _POSIX_VDISABLE;
		term.c_cc[VEOL] = _POSIX_VDISABLE;
		term.c_cc[VERASE] = _POSIX_VDISABLE;
		term.c_cc[VKILL] = _POSIX_VDISABLE;
	}
	if (baudRate >= 0) {
		setBaudRate(&term, baudRate);
	}
	if (dataBits >= 0) {
		term.c_cflag &= ~CSIZE;
		switch(dataBits) {
		case 5:
			term.c_cflag |= CS5;
			break;
		case 6:
			term.c_cflag |= CS6;
			break;
		case 7:
			term.c_cflag |= CS7;
			break;
		case 8:
			term.c_cflag |= CS8;
			break;
		}
	}
	if (parity >= 0) {
		switch(parity) {
		case 0:
			term.c_cflag &= ~PARENB;
			break;
		case 1:
			term.c_cflag |= (PARENB | PARODD);
			break;
		case 2:
			term.c_cflag &= ~PARODD;
			term.c_cflag |= PARENB;
			break;
		}
	}
	if (stopBits >= 0) {
		switch(stopBits) {
		case 1:
			term.c_cflag &= ~CSTOPB;
			break;
		case 2:
			term.c_cflag |= CSTOPB;
			break;
		}
	}
	// returns success if any of the requested changes could be successfully carried out.
	if (tcsetattr(fd, TCSANOW, &term) < 0) {
		RETURN_ERROR(l, strerror(errno))
	}
	RETURN_SUCCESS(l)
}

static int setTimeout(lua_State *l) {
	RETURN_ERROR(l, "Unsupported operation")
}

static int flush(lua_State *l) {
	int fd;
	fd = getFileDesc(l, 1);
	// could be TCIFLUSH, TCOFLUSH or TCIOFLUSH
	if (tcflush(fd, TCIOFLUSH) == -1) {
		RETURN_ERROR(l, strerror(errno))
	}
	RETURN_SUCCESS(l)
}

static int waitDataAvailable(lua_State *l) {
	int fd;
	int waitTimeOut;
	int count;
	fd_set set;
	struct timeval tv;
	struct timeval *ptv;
	fd = getFileDesc(l, 1);
	waitTimeOut = luaL_optinteger(l, 2, -1); // default to no timeout
	FD_ZERO(&set);
	FD_SET(fd, &set);
	if (waitTimeOut < 0) {
		// no timeout
		ptv = NULL;
	} else {
		tv.tv_sec = waitTimeOut / 1000;
		tv.tv_usec = (waitTimeOut % 1000) * 1000;
		ptv = &tv;
	}
	count = select(fd + 1, &set, NULL, NULL, ptv);
	if (count < 0) {
		RETURN_ERROR(l, strerror(errno))
	}
	if (count == 0) {
		RETURN_ERROR(l, "timeout")
	}
	RETURN_SUCCESS(l)
}

static int available(lua_State *l) {
	int fd;
	int count;
	fd_set set;
	struct timeval tv;
	fd = getFileDesc(l, 1);
	FD_ZERO(&set);
	FD_SET(fd, &set);
	tv.tv_usec = 1;
	tv.tv_sec = 0;
	select(fd + 1, &set, NULL, NULL, &tv);
	if (ioctl(fd, FIONREAD, &count) == -1) {
		RETURN_ERROR(l, strerror(errno))
	}
	lua_pushinteger(l, count);
	return 1;
}
