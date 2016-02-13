Hexapod
==========
This is an ongoing project in which I'm making an arduino-based hexapod that is 
controlled via a radio-controller.

**Status:** I've run tests for the radio-controller and for the servos. The prototype 
for the legs of the hexapod is finished and has also been tested. 

**Note:** The signal from the radio-controller is recorded using interrupts instead of 
the standard ```pulseIn```-function as this function can take up to 20ms to return a 
value. This delay is too high when we want to read 6 signals from the radio-controller 
and also control all 18+ servos.
