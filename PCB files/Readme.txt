Gerber File_BOM:

This is a custom layout I did for this setup.
Nothing special, and can be done on breadboard or with just wire(s).
I also added a 5vdc input, as in my testing the LM7805's looked to power everything
without issue, but nothing actually worked unless arduino was plugged in. I suspect
low current, even with 2 in parallel, but may be a ground/pwr issue on my layout.

Folders:
Mr Innovative Board:
PCB from Mr Innovatives link on youtube. I believe this is the smb version, and not all components
are required for this wire cutter project. However I have not tested this board.

Gerber_Ext_5v:  <-- This is what I will use, as a shield for UNO board.
For use with external 5v (input) to power Arduino board & Display.

old:
Original board I made, included limit switch(s) plug (A1SW, A0SW).
I've found the LM7805's (U2, U3) work, but not impressed with stability, low voltage. Plan to run a buck converter
to drop 12v from main supply down to 5v for display/Uno - 5VIN plug (input voltage, not output).
