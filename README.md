# Usb data sender
## CLI tool for sending data to usb devices

This is tool for sending data to usb devices. It uses libusb and allows sending control, bulk or interrupt transfers.

### Example use

`usbdatasender -c 1234:1234 1 80 9 30d 1 8 0 1234abcd`

This sends usb control transfer to device with vendor id of 0x1234 and product id of 0x1234.
