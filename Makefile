RODATA_SEGMENT=RODATA

include Common.mk

objects= \
	newbootldr.o abcmusic.o bg.o logo.o rail0.o rail1.o box.o fail.o smallbox.o smallfail.o gears.o smalltoys.o toys.o select.o elf.o calendar.o main.o

target = santafactory.lnx

all: $(target)

toys.o : toys.bmp
	$(SPRPCK) -t6 -p2 -r006001 -S016016 $< 
	$(ECHO) .global _toys000000 > $*.s
	$(ECHO) .global _toys000001 >> $*.s
	$(ECHO) .global _toys000002 >> $*.s
	$(ECHO) .global _toys000003 >> $*.s
	$(ECHO) .global _toys000004 >> $*.s
	$(ECHO) .global _toys000005 >> $*.s
	$(ECHO) .segment \"$(RODATA_SEGMENT)\" >> $*.s
	$(ECHO) _toys000000: .incbin \"toys000000.spr\" >> $*.s
	$(ECHO) _toys000001: .incbin \"toys000001.spr\" >> $*.s
	$(ECHO) _toys000002: .incbin \"toys000002.spr\" >> $*.s
	$(ECHO) _toys000003: .incbin \"toys000003.spr\" >> $*.s
	$(ECHO) _toys000004: .incbin \"toys000004.spr\" >> $*.s
	$(ECHO) _toys000005: .incbin \"toys000005.spr\" >> $*.s
	$(AS) -t lynx -o $@ $(AFLAGS) $*.s

smalltoys.o : smalltoys.bmp
	$(SPRPCK) -t6 -p2 -r003002 -S008008 $< 
	$(ECHO) .global _smalltoys000000 > $*.s
	$(ECHO) .global _smalltoys000001 >> $*.s
	$(ECHO) .global _smalltoys000002 >> $*.s
	$(ECHO) .global _smalltoys001000 >> $*.s
	$(ECHO) .global _smalltoys001001 >> $*.s
	$(ECHO) .global _smalltoys001002 >> $*.s
	$(ECHO) .segment \"$(RODATA_SEGMENT)\" >> $*.s
	$(ECHO) _smalltoys000000: .incbin \"smalltoys000000.spr\" >> $*.s
	$(ECHO) _smalltoys000001: .incbin \"smalltoys000001.spr\" >> $*.s
	$(ECHO) _smalltoys000002: .incbin \"smalltoys000002.spr\" >> $*.s
	$(ECHO) _smalltoys001000: .incbin \"smalltoys001000.spr\" >> $*.s
	$(ECHO) _smalltoys001001: .incbin \"smalltoys001001.spr\" >> $*.s
	$(ECHO) _smalltoys001002: .incbin \"smalltoys001002.spr\" >> $*.s
	$(AS) -t lynx -o $@ $(AFLAGS) $*.s

gears.o : gears.bmp
	$(SPRPCK) -t6 -p2 -r004001 -S007007 $< 
	$(ECHO) .global _gears000000 > $*.s
	$(ECHO) .global _gears000001 >> $*.s
	$(ECHO) .global _gears000002 >> $*.s
	$(ECHO) .global _gears000003 >> $*.s
	$(ECHO) .segment \"$(RODATA_SEGMENT)\" >> $*.s
	$(ECHO) _gears000000: .incbin \"gears000000.spr\" >> $*.s
	$(ECHO) _gears000001: .incbin \"gears000001.spr\" >> $*.s
	$(ECHO) _gears000002: .incbin \"gears000002.spr\" >> $*.s
	$(ECHO) _gears000003: .incbin \"gears000003.spr\" >> $*.s
	$(AS) -t lynx -o $@ $(AFLAGS) $*.s

$(target) : $(objects)
	$(CL) -t lynx -o $@ -m game.map $(objects) $(others) lynx.lib

clean:
	$(RM) $(objects) $(target) bg.s logo.s rail0.s rail1.s smallbox.s smallfail.s box.s fail.s gears.s smalltoys.s toys.s select.s elf.s calendar.s main.s *.pal *.spr *.map
