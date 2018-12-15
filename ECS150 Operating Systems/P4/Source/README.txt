Tyler Welsh
Patrick Johnston

Used: Piazza, FATLayout.pdf (from piazza), fatgen103.pdf  (provided), and various piazza posts

WHAT'S NOT DONE:
NO EXTRA CREDIT
Segfaulting for some reason after running file.so - can't figure out why.
Writing sectors back when dismounting is NOT implemented, however writing clusters to cache IS.
Unsure about some VMFile____ functions working or not.
A lot of stuff is kind of just brute forced.
VMDirectoryChange is unwritten, just auto fails.
"rm" does not work in shell.so
"mkdir" does not work in shell.so
"cat" does not work in shell.so

WHAT DOES WORK:
Running vm ./shell.so on FAT.ima from Nitta's directory opens and works.
Exiting displays stuff correctly.
ls works.
cd "works".
Running vm ./file.so "works" (seg faults at the end and idk why)
