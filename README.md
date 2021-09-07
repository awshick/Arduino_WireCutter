# Arduino_WireCutter
Arduino wire cutter stripper with nextion display

I'm not a professional. I've worked on my own personal 3d printers for awhile so I'm familiar with arduino and steppers. Never have I done a project like this so its my first from start to finish. I basically saw Mr Innovatives video and wanted a machine similar, but no code or layout was provided. I've made some very simple 3d drawings for the supports and during my trial and error build I decided limit switches where the way I wanted to go. This way I dont care about steps when it comes to cutting the wire (other than feeding wire).
My first nextion HMI - lots of hardships getting through it and the arduino code, again I'm no pro by far. I'm sure there are alot of things that are questionable and/or not correctly done, but I went with what I could figure out.
Please feel free to contribute to the code, if you want to collabrate on this let me know and I'll open the door.
I can be contacted through garzajd3@yahoo.com or message on github.

In the pics below is my version of this tool. I went with adding limit switches on the "cutter" stepper so that I do not need to actually know the # of steps for a close/open.
There are a total of 5 3d printed parts: 1 for the feed stepper mount, 1 for the wire tube holder (adjustable to align with cutter), 1 cutter holder, 1 piece the holds both limits switches (mounted to the cutter stepper). 
I've also opted to use a 5:1 stepper for the cutting unit. I was finding that with a standard 1:1 nema 17 sometimes steps would be skipped when trying to cut wire. I'm sure this can be worked out by adjusting linkage, position, etc but I wanted something that would work consistently with alittle more power. 
![20210907_012745](https://user-images.githubusercontent.com/88321340/132295636-adce5e69-026f-4b7f-a31a-598148319444.jpg)
![20210907_012726](https://user-images.githubusercontent.com/88321340/132295647-d372b304-d155-4083-9c86-81b2a314dd9d.jpg)
![20210906_174530](https://user-images.githubusercontent.com/88321340/132295667-bf88a9e9-61e8-4917-9c3e-27ca65590784.jpg)
