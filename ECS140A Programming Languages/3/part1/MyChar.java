
public class MyChar extends Element 
{
	
	private char value;
	
	public MyChar()
	{
		value = '0';	//Default constructor sets value to 0
	}
	public MyChar(char c)
	{
		value = c;		//Value is set to input
	}
	public char Get()
	{
		return value;	//Return the value of MyChar
	}
	public void Set(char val)
	{
		value = val;	//Set the value of MyChar
	}
	//Abstract print
	public void print()
	{
		System.out.println(value);	//Print the value of MyChar
	}
}
