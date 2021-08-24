# Nodemcu-based-clock-with-automatic-lights.
An NTP server based clock with an oled screen and an led strip lighting.
LED strip is ws2812b uses fastled library.
Oled screen is a 128x32 i2c display.
Main code is in the src folder inside ntpalarmclock.
If using arduino ide ignore the #include <ardiuno.h>.
Circuit diagram(table) is below the video


https://user-images.githubusercontent.com/86282256/130570172-d3131b82-4762-4446-8a79-48a260ff4859.mp4

Circuit:
|Nodemcu        | LED Strip     | OLED  |
| ------------- |:-------------:| -----:|
| D1            |     -         |  SCL  |
| D2            |     -         |  SDA  |
| D4            |   Data        |   -   | 
| 3.3V          |   VCC         |  VCC  |
| GND           |   GND         |  GND  | 
 
