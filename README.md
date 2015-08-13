# minitrash

###6HP teensy 3.1 / wm8731 codec thingie.

![My image](https://farm8.staticflickr.com/7615/17264546385_cec048eaee_z.jpg)

still deliberating a proper name. pjrc audio adapter compatible eurorack module. 

- stereo i/o  : 10 VPP; 44.1k / 16 bit (when using pjrc audio lib)
- 2 x ADC, +/- 5V
- 4 x pots
- 2 x digital inputs
- 2 x illum. tact switches
- microSD card socket
- ext. SPI flash (w25 / soic 8) or SPI SRAM (23LC1024)
- depth: ~ 40mm
 
find the BOM [here](https://github.com/mxmxmx/minitrash/blob/master/hard/BOM.md)
/ preliminary build guide: [here](https://github.com/mxmxmx/minitrash/wiki/Building-it)

- pinout is identical to pjrc audio adapter; being a module, the thing also has:

- 2x CV in    < >   pins A1, A2
- 4x pots     < >   pins A10, A11, A7, A6
- 2x clk in   < >   pins 0, 3
- 2x switches < >   pins 1, 4
- 2x LEDs     < >   pins 2, 5
