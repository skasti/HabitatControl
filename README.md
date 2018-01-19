# HabitatControl
An open source arduino-controlled habitat controller capable of reading temperature, 
humidity and uv-intensity from 3 zones, controlling lighting, heating and rain systems with 4 relays.

Arduino-folder: arduino project and vscode-config. This is where the actual habitatcontrol is.
Nextion-folder User interface project
Eagle-folder (coming): Will contain schematics and board designs.

[You can order finished pcb's from dirtypcb's here](http://dirtypcbs.com/store/designer/details/skasti/5841/arduino-habitat-control-v1-0)

Board is to be used to control lighting/heating/rain-system for reptile habitat, 
but can be used to control anything you can control with relays.

3x Sense-ports (5v, gnd,digital, analog) for inputs
4x Relay outputs with NO* and NC* option
1x Serial-port for connecting to other device (I plan on connecting a Nextion Enhanced display)

Can use separate power supply for relays (shared ground).

I would not advice drawing more than 5A through each channel, 
or about 1kW@230V. 
Most terrarium-equipment seems to be 40-200W, so this is probably well within reason.

* R1-4: 1k
* T1-4: 2N3904
* D1-4: 1N4007
* Relay1-4: SRD-12VDC-SL-C or SRD-5VDC-SL-C

Designed to be used with Arduino Nano as controller

*NO=Normally Open
*NC=Normally Closed
