
public class Map extends Sequence
{
	
	private Sequence map = new Sequence();
	private int pos;
	private int size;
	
	public Map()
	{
		pos = 0;
		size = 0;
		setFirst(map);
	}
	
	public void add(Pair in)
	{
		//Search the map for in.key and enter in ordered manner
		
		int spot = 0;
		
		if(size == 0)
		{
			map.add(in, 0);
			pos = 0;
			size++;
		}
		else if(size == 1)
		{
			MyChar first;
			MyChar second;
			first = (map.top()).getKey();
			second = in.getKey();
			if(first.Get() > second.Get())
			{
				//spot = map.getPos() + 1;
				spot = map.getPos();
				map.add(in, spot);
			}
			else
			{
				spot = map.getPos();
				map.add(in, spot);
			}
			
			size++;
		}
		else
		{
			SequenceIterator stepper;
//			for(stepper = map.begin(); !stepper.equal(map.end()); stepper.advance())
//			{
//				spot++;
//			}
			stepper = map.begin();
//			while(!stepper.equal(map.end()))
//			{
//				if(in.getKey() == ((Pair)(stepper.get())).getKey())
//				spot++;
//				stepper.advance();
//			}
//			char tester = in.getKey().Get();
//			char tester2 = ((Pair)(stepper.get())).getKey().Get();
			
			while(in.getKey().Get() >= ((Pair)(stepper.get())).getKey().Get() && stepper.currentPos != null)
			{
				spot++;
				stepper.advance();
//				tester = in.getKey().Get();
//				tester2 = ((Pair)(stepper.get())).getKey().Get();
			}
			
			map.add(in, spot);
		}
	}
	
	public MapIterator find(MyChar key)
	{
		MapIterator mapper = new MapIterator(map);
		boolean found = false;
		
		while(!found && !(mapper.getIter().equal(map.end())))
		{
			Pair p = mapper.get();
			Character mapKey = p.getKey().Get();
			Character inKey = key.Get();
			if((mapKey.compareTo(inKey) == 0))
			{
				found = true;
			}
			else
			{
				mapper.advance();
			}
		}

		return mapper;
	}
	
	public MapIterator begin()
	{
		MapIterator mep = new MapIterator(map);
		return mep;
	}
	
	public MapIterator end()
	{
		MapIterator mep = new MapIterator(map.back().getNext());
		return mep;
	}
	
	public Sequence getMap()
	{
		return map;
	}

	public void Print()
	{
		map.Print();
	}
}