

public class MyInteger extends Element 
{
	
	private int value;
	
	public MyInteger()
	{
		value = 0;			//Default constructor sets value to 0
	}
	public MyInteger(int i)
	{
		value = i;			//Value is set to input
	}
	public int Get()
	{
		return value;		//Return the value of MyInt
	}
	public void Set(int val)
	{
		value = val;		//Set the value of MyInt
	}
	//abstract print
	public void print()
	{
		System.out.println(value);
	}

}
