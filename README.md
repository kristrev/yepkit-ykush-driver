Yepkit YKUSH Linux kernel driver
================================

The Yepkit YKUSH is a three-port switchable USB hub which also exports a HID
device. By communicating with the HID device via a simple protocol, it is
possible to power the ports up/down individually or all together. The protocol
is as follows:

* Commands 0x01-0x03 powers down port 1-3.
* Command 0x0A powers down all ports.
* In order to power up a port, the command must be OR'd with 0x10.

Host communicates with device using interrupt messages on endpoint 0x1 and each
message must be 6 bytes the long. This two bytes contains the command, in other
words, the command is repeated two times. Timeout is set to 5sec in Yepkit's
example application for controlling the HID device, so I have kept the value.

Since the device is a HID device, the HID driver will bind to it. In order for
the driver to work, you need to unbind from HID first. Another kernel patch for
doing this is incoming, but if you want to test the patch earlier, then you can
follow the instructions for example
[here](https://github.com/vogelchr/led-notify-module).
