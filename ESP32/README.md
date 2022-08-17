# ETAG/NTAG ESP32 chip
This code works in parallel with the Teensy 4.0 "motherboard". The ESP32 is responsible for Bluetooth/WiFi wireless communication, pressure, and
speed data (by way of a Hall Effect sensor). This chip can be updated with over-the-air updates.

## Over the air update
  1.  Power on the chip
  2.  Connect to the chip over Bluetooth
  3.  Send command 'w' over Bluetooth to turn-on WiFi mode (this turns off Bluetooth)
  4.  Connect to the device over Wi-Fi (name of the device will be ETAG\_&#35;&#35;&#35;&#35;&#35;&#35;&#35;)
  5.  In wi-fi browser, connect to 191.168.4.1
  6.  Select the firmware.bin script found in ${PROJECT\_DIRECTORY}/.pio/eps32dev/ folder
  7.  Press the upload button
  8.  Once that is done, the firmware will be updated, the WiFi will turn off and the chip will go back to Bluetooth mode

IMPORTANT: HAVE CHIP BE POWERED FROM USB WHILE THE OVER-THE-AIR INTERFACE IS WORKING, AS A POWER INTERRUPTION CAN BRICK THE CHIP
