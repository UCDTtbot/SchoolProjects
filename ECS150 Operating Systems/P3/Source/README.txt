ECS150 Project 3:
Tyler Welsh
Patrick Johnston

What doesn't work 100%:
Free Chunk Merging is broken, ran out of time.
	The shared memory chunks are not being merged together correctly
	and there are possible duplicate chunk creations.
512 bit passing isn't fully implemented, ran out of time.

Questionably Working:
Thread queueing for lack of allocation space:
	I have it all setup but because deallocation of chunks, and freeing of chunks
	is not fulling working, I have no full way to test this
	
What does work:
All previous VM .so files run, which means shared memory is mostly working.
All memory pooling and allocation is working, except for the deallocation of
	the shared memory space.