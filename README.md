# This project was created to emulate the circuitry and displays used in the Akai AX-80 Polysynth.

I recently purchased both the Akai AX-80 processor board and voice board and managed to get them working to create a polysynth. 

Unfortunately I didn't have the nice AX-80 displays of the 1980's, so I decided that I could decode the original address and data lines of the AX-80 with modern micros and drive OLED screens using the online schematics.

I wanted to get as close to the original size of the AX-80 screens so they can be used as replacements for the AX-80, unfortunately there are not many long narrow screens with reasonable resolution available. I found a mono screen that was 3.12" long with an SPI interface and SSD1322 driver that is supported with the UG8 library.

![Synth](Photos/synth.jpg)

The screen has a resolution of 256x64 which can easily accomodate the 13 blocks x 9 columns of the original VFD displays, but as it is monochrome then I cannot reproduce the orange selector block at the bottom of each parameter and have opted for a filled rectangle when selected and an empty rectangle when not. You may want to experiment with a filter over the lower portion of the screen, but I have found that not very successful.

The schematics show the decoder logic and level conversion from 5V to 3.3v of the original signals, this generates 32 interrupt lines which can be read by 5 ESP32 boards, the 13 address lines carry the segments to be displayed, I simply capture these with a pair of shift registers every time an interrupt is received and display it on the screen, Each ESP-32 is designed to display upto 8 columns like the original AX-80 VFD displays, but between 5 and 8 are used on each display depending on which section of the synth is it being used for. Any unused interrupts can be tied to +3.3v. This makes the code generic for each screen you emulate and just by changing LABEL_SET you can set it for screen 1-5 (0-4). I used ESP32 boards because they are cheap and easily accessible, I have never used them before, but they have Arduino compatibility which I required for the RoxMux library used to capture the 13 address lines. 

Videos of the Displays in action

https://youtu.be/0UIXME1TG3U

https://youtu.be/lRL6-849DsE
