serial for luajls

this lua module provides functions to deal with serial port settings such as bit rates (speed) data bits, parity.

the device/file manipulation such as open, close, read and write are not provided.

```lua
local serialLib = require('serial')
local baudRate = 9600
local dataBits = 8
local stopBits = 1
local parity = 0
local portName = '/dev/ttyUSB0'
--local portName = '\\\\.\\COM3' -- Windows
local fd = io.open(portName, 'r+')
serialLib.flush(fd)
serialLib.setSerial(fd, baudRate, dataBits, stopBits, parity)
fd:read(1)
fd:close()
```
