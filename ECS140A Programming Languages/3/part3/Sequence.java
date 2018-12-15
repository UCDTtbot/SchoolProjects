//Sequence acts as a node for a linked list
public class Sequence extends Element
{	
	//private Element elementType;	//Type of current element in sequence
	//private Sequence seq;				//Holds the sequence itself to act as a linked list

	//FOR EACH ELEMENT
	private Element data;			//Data for the current element such as MyChar MyInteger
	private Sequence Next;
	private int position;
	
	//FOR THE SEQUENCE OVERALL
	private Sequence first;			//Next element
	private Sequence last;			//Previous element
	private int size;				//Size of sequence
	
	public Sequence() //Initialize all to null/0
	{
		//elementType = null;
		data = null;
		Sequence Next = null;
		first = null;
		last = null;
		position = 0;
		size = 0;
	}
	
	public void Print()
	{
		Sequence stepper = first;
		//Surround each sequence in brackets
		System.out.print("[");
		while(stepper.Next != null )
		{
			System.out.print(" ");
			stepper.data.Print();
			stepper = stepper.Next;
		}
		System.out.print(" ");
		stepper.data.Print();
		System.out.print(" ]");
	}
	
	
	public Element first() //Dirty programming. Needs to make sure lastEl is ONLY null for the head of the list
	{
		return first.data;
	}
	
	public Sequence rest()
	{
		//Alright, the way I setup my Sequence, its impossible for me to
		//Not make a new Sequence without changing a lot of the code
		if(first.Next == null)
		{
			return (first);
		}
		first.Next.first = first.Next;
		first.Next.last = this.last;
		setSize(first.Next);
		return (first.Next);
	}
	
	public int length()
	{
		//return size;
		setSize(first);
		return first.size;
	}
	
	public void add(Element elm, int pos)
	{
		//Check is pos is out of bounds
		if(!(pos >= 0) && !(pos < size + 1))
		{
			System.err.println("Position is out of bounds");
			System.exit(1);
		}
		
		if(size == 0)
		{
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(null);
			newData.setPos(pos);
			first = newData;
			last = newData;
			size++;
		}
		else if(size == 1 && pos == first.position)
		{
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(first);
			newData.setPos(pos);
			first.setPos(1);
			last = first;
			first = newData;
			size++;
		}
		else if(size == 1 && pos != first.position)
		{
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(null);
			newData.setPos(pos);
			first.setNext(newData);
			last = newData;
			size++;
		}
		else if(pos == this.size)
		{
			Sequence seq = first;
			Sequence newData = new Sequence();
			while((pos - 1) != (seq.position))
			{
				seq = seq.Next;
			}
			newData.setData(elm);
			newData.setNext(null);
			newData.setPos(pos);
			last = newData;
			size++;
			seq.setNext(newData);
		}
		else if(pos == first.position && size > 1)
		{
			Sequence seq = first;
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(seq);
			newData.setPos(0);
			first = newData;
			size++;
			incPos(newData, pos);
		}
		else
		{	
			//Find object (say seq) at position pos by following next
			//Create a new sequence object (say nw)
			//Copy content of seq into nw, and next now references nw
			//Content of element elm and point to nw
			Sequence seq = first;
			Sequence newData = new Sequence();
			while((pos-1) != seq.position && pos < size)
			{
				seq = seq.Next;
			}
			newData.setNext(seq.Next);
			seq.setNext(newData);
			newData.setData(elm);
			newData.setPos(pos);
			size++;
			incPos(newData, pos);
//			Sequence stepper = newData.Next;
//			while(stepper != null)
//			{
//				pos++;
//				stepper = stepper.Next;
//			}
			//This should work?
		}
		setSize(first);
	}
	
	public void delete(int pos)
	{
		if(pos == 0)
		{
			Sequence seq = first;
			first = first.Next;
			seq.setData(null);
			seq.setNext(null);
			seq.setPos(-15);
			decPos(pos);
			size--;
		}
		else
		{
			Sequence seq = first;
			Sequence newData = new Sequence();
			while((pos-1) != seq.position && pos < size)
			{
				seq = seq.Next;
			}
			newData = seq.Next;
			seq.setNext(seq.Next.Next);
			newData.setData(null);
			newData.setNext(null);
			newData.setPos(-10);
			newData = first;
			decPos(pos);
			size--;
		}
		setSize(first);
	}
		
	public void setData(Element in)
	{
		data = in;
	}
	
	public void setNext(Sequence in)
	{
		Next = in;
	}
	
	public void setPos(int in)
	{
		position = in;
	}
	
	public void incPos(Sequence newData, int pos)
	{
		
		Sequence stepper = newData.Next;
		while(stepper != null)
		{
			stepper.position++;
			stepper = stepper.Next;
		}
	}
	
	public void decPos(int pos)
	{
		Sequence stepper = first;
		if(pos != 0)
		{
			while((pos - 1) != stepper.position)
			{
				stepper = stepper.Next;
			}
		}
		while(stepper != null)
		{
			stepper.position--;
			stepper = stepper.Next;
		}
	}
	
	public void setSize(Sequence head)
	{
		head.size = 0;
		Sequence stepper = head;
		while(stepper.Next != null)
		{
			head.size++;
			stepper = stepper.Next;
		}
		head.size++;
	}
	
	//PART 3 STUFF
	public Element index(int pos)
	{
		Sequence stepper = first;
		while(pos != stepper.position)
		{
			stepper = stepper.Next;
		}
		
		return stepper.data;
	}
	
	public Sequence copy()
	{
		Sequence nw = new Sequence();
		Sequence stepper = this.first;
		int x;
		for(x = 0; x < this.length(); x++)
		{
			if(!(stepper.data instanceof Sequence))
			{
				//nw.add(stepper.data, x);
				if(stepper.data instanceof MyInteger)
				{
					//nw.add(((MyInteger)Stepper.data).Get(), x);
					MyInteger toAdd = new MyInteger(((MyInteger)stepper.data).Get());
					nw.add(toAdd, x);	
				}
				if(stepper.data instanceof MyChar)
				{
					//nw.add(((MyInteger)Stepper.data).Get(), x);
					MyChar toAdd = new MyChar(((MyChar)stepper.data).Get());
					nw.add(toAdd, x);	
				}
			}
			else if (stepper.data instanceof Sequence)
			{
				Sequence recurse = (Sequence) stepper.data;
				nw.add(recurse.copy(), x);
			}
			stepper = stepper.Next;
		}
		return nw;
	}
	
	public Sequence flatten()
	{
		Sequence Stepper = this.first;
		Sequence nw = new Sequence();
		int index = 0;
		int x;
		for(x = 0; x < this.length(); x++)
		{
			if(!(Stepper.data instanceof Sequence))
			{
				nw.add(Stepper.data, index);
				index++;
			}
			else if(Stepper.data instanceof Sequence)
			{
				Sequence newThing = ((Sequence)Stepper.data).flatten();
				int y;
				int length = ( newThing.length());
				for(y = 0; y < length; y++)
				{
					if(newThing.data == null)
					{
						nw.add(newThing.first.data, index);
						newThing = newThing.first.Next;
					}
					else
					{
						nw.add(newThing.data, index);
						newThing = newThing.Next;
					}
					
					index++;
				}
			}

			Stepper = Stepper.Next;
		}
		
		return nw;
	}	
}


