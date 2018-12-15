Tyler Welsh (912341695)
Patrick Johnston

Websites Used: 
Various references 	http://www.cplusplus.com/reference/
Various man pages:	http://www.die.net/
					http://linux.die.net/man/
Various examples:	http://codewiki.wikidot.com/c
execvp help:		http://www.albany.edu/~csi402/pdfs/handout_13.4.pdf
					http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html
pipe help:			http://www2.cs.uregina.ca/~hamilton/courses/330/notes/unix/pipes/pipes.html
					http://www.tldp.org/LDP/lpg/node11.html
dir help:			http://www.badprog.com/unix-gnu-linux-system-calls-readdir
to_string adaption: http://stackoverflow.com/questions/12975341/to-string-is-not-a-member-of-std-says-so-g
exit code: 			http://stackoverflow.com/questions/3659616/returning-exit-code-from-child
stats help:			http://codewiki.wikidot.com/c:system-calls:stat
replace:			http://stackoverflow.com/questions/2896600/how-to-replace-all-occurrences-of-a-character-in-string

Working as of 4/12 10:16pm:
All read/write to console working correctly
Directory display at all times correctly
Up/Down arrow working with minimal functionality. Down arrow is breaking.
CD works correctly. Have not tested with redirect.
LS works correctly. Have tested with (>) redirect. Not sure on (<).
PWD works as intended. Redirection works as far as I know.
HISTORY works as intended with redirection.
EXIT works (thank god).
EXECVP works for at least make.
Forking works.
Everything parses correctly.
No error checking was implemented
Piping is barely implemented. A general understanding of piping exists
	see the comment in the tryFork function ***

Personal Notes(Tyler):
I waited way too long to finish this. Personally over 20 hours just working Saturday
and Sunday. Got as much working as possible in such a short amount of time.
Figuring out I/O to console and parsing took most of the time.