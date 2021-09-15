Gerber File_BOM:

This is a custom layout I did for this setup.
Nothing special, and can be done on breadboard or with just wire(s).
I'm still waiting to recieve PCB to verify I did not miss something.
I also added a 5vdc input, as in my testing the LM7805's looked to power everything
without issue, but nothing actually worked unless arduino was plugged in. I suspect
low current, even with 2 in parallel, but may be a ground/pwr issue on my layout.


Gerber_Ext_5v:  <-- This is what I will use
This PCB is same size as UNO, and for use with external 5v to power uno.
I've found the LM7805's work, but not impressed with stability, low voltage. Plan to run a buck converter
to drop 12v from main supply down to 5v for display/Uno VIN.
