

public class MyInteger extends Element 
{
	
	private int value;	//value of int
	
	public MyInteger()	//Default, initialize to 0
	{
		value = 0;
	}
	public MyInteger(int i) //Initialize to input i
	{
		value = i;
	}
	public int Get() //Function to get value
	{
		return value;
	}
	public void Set(int val)	//Function to set value
	{
		value = val;
	}
	//abstract print
	public void Print()	//Print out value as is
	{
		System.out.print(value + "");
	}

}
