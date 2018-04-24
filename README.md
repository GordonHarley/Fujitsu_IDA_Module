# IDA Fujitsu-FR Processor Module
## Notes from original editor
### What is this?
This is the improved FR plugin for IDA Pro 6.x, that fixes a number of the problems in the default processor and adds switch statement detection and type information.

## Authors
* Un-named author of original IDA SDK module.  Ilfak or one of the IDA guys maybe?  A very bored unnamed assembly jockey?
* Simeon Pilgrim -- see some of his other fun items
  *  [on github right here](https://github.com/simeonpilgrim).
* Rule 34 of ancient Pentax hacking days for giving me issues he was still having.
* GordonHarley


You may find the [original tree here](https://github.com/simeonpilgrim/nikon-firmware-tools/tree/master/FR%20IDA%20Plugin) if something goes terribly wrong with the partial clone.

## My Notes
Fujitsu FR (prior to FR81) relies very heavily on register-indirect addressing and ldi:32 / ldi:20 Rx, #imm -- call @Rx constructs.  This get more complicated when you throw indirect jump tables or roms with various multiple interrupt vector tables all over the place.  Add that to a mostly 2 byte instruction length and nearly everything gets decoded as some instruction, much to the horror of everyone reading it.

### Some history
I once pointed out some bugs in this module as an aside while I was beta testing some version of IDA or another.  Ilfak is always quick to respond, and his take was that nobody really cared about it, so fixes were unlikely.

I always wanted to improve on it but I'd given up on my advisory role in the Pentax K10D hacking attempt years before and didn't have the time or motivation.

### Additions
Upcoming;  this is the initial checkin of the modified plugin version (sorry about lack of history).

### Compiling
Visual studio 2017 works fine, and the project has been updated to both use it and to live in the original structure of the IDA SDK _(i.e. /idasdk6x/module/fr/)_.  You'll obviously need a license of IDA for this project, either standard or pro edition.  I only have a Windows license so I can't say whether this will build at all on Mac / Linux in the current state.  Tested with an in-between version pulled from their build tree somewhere between v6.9 and an update patch because I accidentally the files.

You may need to mess with the solution a litle, but hey.
