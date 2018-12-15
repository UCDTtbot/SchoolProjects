//Sequence acts as a node for a linked list
public class Sequence extends Element
{	
	private Element elementType;	//Type of current element in sequence
	private Element data;			//Data for the current element
	private Element nextEl;			//Next element
	private Element lastEl;			//Previous element
	private int size;				//Size of sequence
	
	public Sequence()
	{
		elementType = null;
		data = null;
		nextEl = null;
		lastEl = null;
		size = 0;
	}
	
	public void print()
	{
		System.out.println("Sequence: " );
	}
}


