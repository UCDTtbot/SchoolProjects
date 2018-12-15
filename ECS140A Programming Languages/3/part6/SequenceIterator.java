
public class SequenceIterator 
{
	Sequence head;					//Head of the iterator
	Sequence currentPos;			//Current position of the iterator
	private int index;				//Index of where we are/int representation of currentPos
	
	public SequenceIterator()
	{
		head = null;			//Default constructor sets our sequence to null
		currentPos = head;
		index = 0;
	}
	
	public SequenceIterator(Sequence in)
	{
		head = in;			//Point the head of the Iterator to input, and set currentPos to head
		currentPos = head;
		index = 0;
	}
	
	public boolean equal (SequenceIterator other)
	{
		boolean flag = false;				
		if(currentPos == other.currentPos)		//If out currentPos is equal to input's, flag it to true and return flag
		{
			flag = true;
		}
		return flag;
	}
	
	public void advance()
	{
		if(currentPos == null)			//Case 1 if our currentPos is null, return null
		{
			currentPos = null;
		}
		else if(currentPos.length() == 1)		//If the length of the current Sequence pointed to is only 1, the next position will be null so return null
		{
			currentPos = null;
		}
		else
		{
			currentPos = currentPos.rest();	//Advancing forward is the same as calling the rest of the sequence, so just call .rest()
			index++;
		}

	}
	
	public Element get()
	{
		Element e = head.index(index);	//Easiest way to find would be used index, which returns the element at index x
		return e;						//Index is always set to be the position of current position, so returns index of currentPos
	}
	
	public Sequence getPos()
	{
		return currentPos;		//Get position of iterator
	}
	
	
}
