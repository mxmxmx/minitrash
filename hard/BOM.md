
BOM --- minitrash
=================



A. core pcb:
=================


ICs/regulators/etc:

- wm8731 (SSOP 28) : 1 x (mouser # 238-WM8731SEDS/V)
- TL072 (SOIC-8): 3 x (or use something fancier for audio i/o; in which case: 1 x TL072 + 2 x something fancier)
- ADP150 (3v3 regulator, TSOT): 1 x (mouser # 584-ADP150AUJZ-3.3R7)
- SM5817 diodes : 2 x (mouser # 833-SM5817PL-TP)
- LM1117-50 (5v0 LDO reg., SOT-223): 1 x (mouser # 511-LD1117S50)
- lm4040-5v0 (sot-23) : 1 x
- teensy3.1 : 1 x (don't forget to cut the usb/power trace)

resistors (0603):

- 100R : 2 x
- 120R : 2 x
- 510R : 1 x
- 910R : 2 x
- 1k   : 2 x
- 4k7  : 2 x
- 5k6  : 2 x
- 27k  : 4 x
- 33k  : 2 x (*)
- 47k  : 2 x
- 100k : 6 x (*)

caps, SMD:

- 15p (NP0/C0G): 2 x (0603) 
- 10n (NP0/C0G): 2 x (0805 or 1206)
- 68n (NP0/C0G): 2 x (0805)
- 100n : 22 x (0603) 
- 10uF : 5 x (0805) (ceramic or tantalum)


electrolytic caps, through-hole:

- 22uF (35V or better): 2 x (power decoupling)
- 10uF (16V or better): 2 x (in audio path/AC coupling)


misc:

- 2x5 pin pin header (euro power connector): 1 x
-  3-pin single row precision ("machined" / "round") socket (female), RM 2.54mm : 1 x
-  5-pin single row precision ("machined" / "round") socket (female), RM 2.54mm : 2 x
- 10-pin single row precision ("machined" / "round") socket (female), RM 2.54mm : 1 x
- 14-pin single row precision ("machined" / "round") socket (female), RM 2.54mm : 2 x
-  3-pin single row precision ("machined" / "round") pin header  (male), RM 2.54mm : 1 x
- 14-pin single row precision ("machined" / "round") pin headers (male), RM 2.54mm : 2 x


optional: 

- microSD socket (molex) : 1 x (# 502774-0891, mouser # 538-502774-0891)
- SOIC-8 spi flash (W25), e.g. winbond W25Q128FV.
- alternatively, SOIC-8 spi RAM, notably microchip 23LC1024, does fit, too. 
- lm4040-2v5 or lm4040-3v0 (sot-23, for teensy 3.x AREF) (NB: in this case, some resistor values need adjustment. see note below)


B. control pcb:
=================

- NPN transistors (MMBT3904, SOT-23) : 2 x

0603 resistors:

- 0R   : 1 x (or use piece of wire)
- 470R : 2 x (adust, depending on LED)
- 510R : 2 x (or use 470R, value is non-critical)
- 33k  : 2 x (may be 0805)
- 100k : 2 x (may be 0805)

0603 caps:

- 100n : 2 x

0805 caps: 

- 1uF  : 4 x (may be 1206)

misc: 

- 1k-10k 9mm pots, vertical "tall trimmer" / song huei : 4 x 
- jacks, thonkiconn : 8 x
- tact switches, illuminated - Highly PB6149L-X (X being the colour) or e-switch TL1265 : 2 x (†)
- M3 spacer/standoff 10mm : 1 x
- M3 screws, 5mm : 2 x
-  5-pin single row precision ("machined" / "round") pin headers (male), RM 2.54mm : 2 x
- 10-pin single row precision ("machined" / "round") pin headers (male), RM 2.54mm : 1 x

notes:
=================

( * ) if using a LM4040 for AREF, the CV range needs adjustment. for instance, when using a lm4040-3v0, the effective range reduces to 0 - 3v0. to compensate, use 110k resistors instead. the CV inputs are inverting op amps, so 33/110 * 10V = 3.0V; ditto for lm4040-2v5 -- in this case, 30k and 120k would get you there (30/120 * 10V = 2.5V)

( † ) e.g. mouser # 612-TL1265YQSCLR (yellow), 612-TL1265BQSCLR (blue), 612-TL1265RQSCLR (red), 612-TL1265GQSCLR (green)
