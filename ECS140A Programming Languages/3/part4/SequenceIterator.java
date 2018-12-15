
public class SequenceIterator 
{
	Sequence head;
	Sequence currentPos;
	private int index;
	
	public SequenceIterator()
	{
		head = null;
		currentPos = head;
		index = 0;
	}
	
	public SequenceIterator(Sequence in)
	{
		head = in;
		currentPos = head;
		index = 0;
	}
	
	public boolean equal (SequenceIterator other)
	{
		boolean flag = false;
		if(currentPos == other.currentPos)
		{
			flag = true;
		}
		return flag;
	}
	
	public void advance()
	{
		if(currentPos == null)
		{
			currentPos = null;
		}
		else if(currentPos.length() == 1)
		{
			currentPos = null;
		}
		else
		{
			currentPos = currentPos.rest();
			index++;
		}

	}
	
	public Element get()
	{
		Element e = head.index(index);
		return e;
	}
	
	
}
