
public class MyChar extends Element 
{
	
	private char value; //Value of char
	
	public MyChar()	//Default, initialize to '0'
	{
		value = '0';
	}
	public MyChar(char c) //Initialize to c
	{
		value = c;
	}
	public char Get() //Function to return value
	{
		return value;
	}
	public void Set(char val)	//Function to set value
	{
		value = val;
	}
	//Abstract print
	public void Print()	//Prints out value surrounded by ''
	{
		System.out.print("'"+value+"'" );
	}
}
