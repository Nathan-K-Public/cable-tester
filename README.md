# cable-tester
Arduino Cable Tester based on Mega2560

> Project development collection can be found here on Google Plus:

> https://plus.google.com/collection/YVyBSB

> Video of tester in action can be seen here:

> https://plus.google.com/102612254593917101378/posts/ViVuKPyG9Dn

> Assembly guide with pictures for version (v2.0-xxx) with Rp testing addon can be found here:

> https://plus.google.com/102612254593917101378/posts/M9emESx9iTr

This is an automated 52x52 way cable conductivity tester. It is fully modular and allows customizable  breakout board definitions and pin subtypes. It also has enhanced functionality to do Rp/Ra testing + Live monitoring mode for USB-C testing and cable/charger validation.

Revision v2.0 is a major departure from the v1.0 design and requires NO SOLDERING. (Or minimal, if automated Rp/Ra testing is desired.) This update also includes better documentation on how to assemble the device and Rp bypass resistors. This will be improved with time as I figure out Git. (See the link above.) Plaintext code commentary has also been added to help explain the approaches used for novices.

Please note this leaves pins necessary for Adafruit Joystick TFT/SPI/I2C/Serial port #1 exposed. You may need to reroute the SPI pins 50-54 (ATMega location to Uno location) and shift pins 13-9 elsewhere, but it is possible to retain SPI and I2C functionality.

> A list from JameCo electronics (MINUS the breakouts) can be found in this directory.

> A list from Amazon (INCLUDING some breakouts) can be found below. But parts tend to vanish mysteriously.
https://amzn.com/w/U4M5LVB4DDW
