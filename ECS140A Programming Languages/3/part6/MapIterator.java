
public class MapIterator extends SequenceIterator
{
	private SequenceIterator iter;
	private int index;
	
	public MapIterator()
	{
		iter = new SequenceIterator();
	}
	
	public MapIterator(Sequence in)
	{
		iter = new SequenceIterator(in);
	}
	
	public boolean equal (MapIterator other)
	{
		boolean flag = false;
		if(iter.getPos() == other.iter.getPos())
		{
			flag = true;
		}
		return flag;
	}
	
	public void advance()
	{
		iter.advance();
	}
	
	public Pair get()
	{
		Pair p = (Pair)iter.get();
		return p;
	}
	
	public SequenceIterator getIter()
	{
		return iter;
	}
}