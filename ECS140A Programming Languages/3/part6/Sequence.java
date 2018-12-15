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
	
	public Sequence() //Initialize all to null//
	{
		data = null;
		Sequence Next = null;	
		first = null;
		last = null;
		position = 0;
		size = 0;
	}
	
	public void Print()
	{
		Sequence stepper = first;			//Stepper to step through the sequence
		//Surround each sequence in brackets	//Was made before Sequence Iterator, iterator would work here
		System.out.print("[");
		while(stepper.Next != null )	//Print out contents of sequence while the next Element isn't null
		{
			System.out.print(" ");
			stepper.data.Print();
			stepper = stepper.Next;
		}
		System.out.print(" ");
		stepper.data.Print();
		System.out.print(" ]");
	}
	
	
	public Element first() //Returns the data of the first element 
	{
		return first.data;
	}
	
	public Sequence rest()
	{
		//If the next element is null, aka our sequence is only 1 element big, return the first
		if(first.Next == null)
		{
			return (first);
		}
		first.Next.first = first.Next;	//Set the 2nd element's first equal to first's next
		first.Next.last = this.last;	//Set the 2nd element's last equal to overall last
		setSize(first.Next);	//Set the size
		return (first.Next);	//Return the 2nd element
			//Explanation is a little confusing
	}
	
	public int length()
	{
		//return size;
		setSize(first);	//setSize here as size is always changing and not changed on the go
		return first.size;	//guarantees size is always right but hurts running time
	}
	
	public void add(Element elm, int pos)
	{
		//Check is pos is out of bounds
		if(!(pos >= 0) && !(pos < size + 1))
		{
			System.err.println("Position is out of bounds");
			System.exit(1);
		}
		
		//Each case basically does the same thing, but we need a lot of cases
		
		if(size == 0) //if sequence is empty, add to first
		{
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(null);
			newData.setPos(pos);
			first = newData;
			last = newData;
			size++;
		}
		else if(size == 1 && pos == first.position)	//If sequence has one item, and we want to insert into the front
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
		else if(size == 1 && pos != first.position)	//If sequence has one item, and we want to insert after first
		{
			Sequence newData = new Sequence();
			newData.setData(elm);
			newData.setNext(null);
			newData.setPos(pos);
			first.setNext(newData);
			last = newData;
			size++;
		}
		else if(pos == this.size)	//If wanting to stick onto the very end, position is equal to size
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
		else if(pos == first.position && size > 1)	//If sequence is size 2 or more, and we want to enter in the first position
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
		else	//All other cases, meaning between positions 1 and last
		{	
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
		}
		setSize(first);
	}
	
	public void delete(int pos)
	{
		//Very similar to add, but with less cases.
		//Basically just moves around the .Next of each element and lets the now unhooked element just fall out of scope
		if(pos == 0)	//If deleting the front
		{
			Sequence seq = first;
			first = first.Next;
			seq.setData(null);
			seq.setNext(null);
			seq.setPos(-15); //Arbitrary negative position
			decPos(pos);
			size--;
		}
		else if(pos == (length()-1))	//If deleting the last element
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
			size--;
		}
		else	//Deleting any other element
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
		
	public void setData(Element in)		//Needed to set data out of sequence
	{
		data = in;
	}
	
	public void setNext(Sequence in)	//Needed to set Next out of sequence
	{
		Next = in;
	}
	
	public Sequence getNext()			//Get the Next out of Sequence
	{
		return this.Next;
	}
	
	public void setPos(int in)		//Set Position out of Sequence
	{
		position = in;
	}
	
	public void incPos(Sequence newData, int pos)
	{
		
		Sequence stepper = newData.Next;	//Increment the position of each element after the given Sequence position
		while(stepper != null)	
		{
			stepper.position++;
			stepper = stepper.Next;
		}
	}
	
	public void decPos(int pos)		//Decrement the position of each element after position
	{
		Sequence stepper = first;
		if(pos != 0)
		{
			while((pos - 1) != stepper.position)	//Step to where we want to decrement
			{
				stepper = stepper.Next;
			}
		}
		while(stepper != null)	//Decrement
		{
			stepper.position--;
			stepper = stepper.Next;
		}
	}
	
	public void setSize(Sequence head)	//Set the size of the Sequence
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
	
	//////////PART 3 STUFF////////////
	public Element index(int pos)	//Return the data at given index
	{
		Sequence stepper = first;
		if(pos == this.length())	//Return end
		{
			return last.data;
		}
		while(pos != stepper.position)	//While given position is not Sequence position, iterate through
		{
			stepper = stepper.Next;
		}
		
		return stepper.data;
	}
	
	public Sequence copy()	//Create a new, exact deep copy of our sequence, creating new elements for each thing
	{
		Sequence nw = new Sequence();
		Sequence stepper = this.first;
		int x;
		for(x = 0; x < this.length(); x++)
		{
			if(!(stepper.data instanceof Sequence))	//If current element isn't a sequence, add accordingly
			{
				if(stepper.data instanceof MyInteger)
				{
					MyInteger toAdd = new MyInteger(((MyInteger)stepper.data).Get());	//Create a whole new Item to add
					nw.add(toAdd, x);	
				}
				if(stepper.data instanceof MyChar)
				{
					MyChar toAdd = new MyChar(((MyChar)stepper.data).Get());	//Create a whole new Item to add
					nw.add(toAdd, x);	
				}
			}
			else if (stepper.data instanceof Sequence)
			{
				Sequence recurse = (Sequence) stepper.data;	//Call recursively to unwrap the Sequence into elements
				nw.add(recurse.copy(), x);
			}
			stepper = stepper.Next;
		}
		return nw;
	}
	
	public Sequence flatten()	//Tricky implementation. Needs to flatten out/unwrap the sequences that occur
	{							//Creates a new sequence that links to the original but everything is unwrapped so no sequence of sequences
		Sequence Stepper = this.first;
		Sequence nw = new Sequence();
		int index = 0;
		int x;
		for(x = 0; x < this.length(); x++)
		{
			if(!(Stepper.data instanceof Sequence))
			{
				nw.add(Stepper.data, index);	//If data isn't a sequence, just add it
				index++;
			}
			else if(Stepper.data instanceof Sequence)
			{
				Sequence newThing = ((Sequence)Stepper.data).flatten();	//Call flatten recursively if the data is a Sequence
				int y;
				int length = ( newThing.length());
				for(y = 0; y < length; y++)
				{
					if(newThing.data == null)
					{
						nw.add(newThing.first.data, index);	//Add the data obtained from flattening
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
	
	////////////////PART 4////////////////	
	public SequenceIterator begin()		//Sequence iterator Begin at this
	{
		SequenceIterator seq = new SequenceIterator(this);
		return seq;
	}
	
	public SequenceIterator end()
	{
		SequenceIterator seq = new SequenceIterator(this.last.Next);	//Sequence iterator end after last (null)
		return seq;
	}
	
	///////////////PART 5/////////////////////////////////////////
	public MyChar getKey()		//For pairs, get the key
	{
		Pair p = new Pair();
		p = ((Pair)data);
		return p.getKey();
	}
	
	public Sequence top()	//return the first element
	{
		return first;
	}
	
	public Sequence back()	//return the last element
	{
		return last;
	}
	
	public int getPos()	//return the current position
	{
		return position;
	}
	
	public void setFirst(Sequence in)	//Set the first to input
	{
		first = in;
	}
}


