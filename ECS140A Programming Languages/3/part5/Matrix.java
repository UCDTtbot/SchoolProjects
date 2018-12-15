
public class Matrix extends Sequence
{
	
	Sequence matrixArray = new Sequence();
	int rowSize;
	int colSize;
	int arraySize;
	
	public Matrix(int rowsize, int colsize)
	{
		//Creates matrix of size rowsize x colsize sets all to zero
		arraySize = rowsize * colsize;
		rowSize = rowsize;
		colSize = colsize;
		MyInteger e = new MyInteger(0);
		for(int i = 0; i < arraySize; i++)
		{
			matrixArray.add(e, i);
		}
	}
	
	public void Set(int rowPos, int colPos, int value)
	{
		//Set value of element
		MyInteger toAdd = new MyInteger(value);
		matrixArray.add(toAdd, rowPos*colSize + colPos);
	}
	
	public int Get(int rowPos, int colPos)
	{
		MyInteger item = (MyInteger)matrixArray.index(rowPos*colSize + colPos);
		return item.Get();	//Get value of element
	}
	
	public Matrix Sum(Matrix mat)
	{
		if(!(arraySize == mat.arraySize))
		{
			System.out.println("Matrix dimensions incompatible for Sum");
			System.exit(1);
		}
		
		Matrix newM = new Matrix(rowSize, colSize);
		int adder;
		for(int x = 0; x < rowSize; x++)
		{
			for(int y = 0; y < colSize; y++)
			{
				adder = (Get(x,y) + mat.Get(x,y));
				newM.Set(x, y, adder);
			}
		}
		return newM;	//returns sum of matrices mat and this
	}
	
	public Matrix Product(Matrix mat)
	{
		Matrix newM = new Matrix(rowSize, mat.colSize);
		if(!(colSize == mat.rowSize))
		{
			System.out.println("Matrix dimensions incompatible for Product");
			System.exit(1);
		}
		else
		{
			int adder = 0;
			for(int x = 0; x < rowSize; x++)
			{
				for(int y = 0; y < mat.colSize; y++)
				{
					adder = 0;
					for(int inner = 0; inner < colSize; inner++)
					{
						adder  += (Get(x, inner) * mat.Get(inner, y));
						newM.Set(x, y, adder);
					}
				}
			}
		}
		return newM;	//returns the product of two matrices mat and this
	}
	
	public void Print()
	{
		//Print full matrix
		
		for(int x = 0; x < rowSize;x++)
		{
			System.out.print("[");
			for(int y = 0; y < colSize; y++)
			{
				System.out.print(" " + Get(x, y));
			}
			System.out.print(" ]");
			System.out.println();
		}
		
	}
}