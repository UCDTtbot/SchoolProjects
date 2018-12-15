
public class Pair extends Element
{
	private Element value;	//Value of the pair
	private MyChar key;		//Key of the pair
	
	public Pair()	//Default sets to null
	{
		value = null;
		key = null;
	}
	
	public Pair(MyChar k, Element v)	//Set key and value to inputs
	{
		value = v;
		key = k;
	}
	
	public MyChar getKey()	//return the key
	{
		return key;
	}
	
	public void Print()	//Print out pairs
	{
		System.out.print("(");
		key.Print();
		System.out.print(" ");
		value.Print();
		System.out.print(")");
	}
}