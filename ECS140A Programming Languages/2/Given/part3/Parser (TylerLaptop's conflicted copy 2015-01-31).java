/* *** This file is given as part of the programming assignment. *** */

public class variable{

	private String variable_name;
	private int data;
	
	private void setData(String name, int dataIn)
	{
		variable_name = name;
		data = dataIn;
	}
	
	private void getData()
	{
		return data;
	}
	private void getName()
	{
		return variable_name;
	}
	
}


public class Parser {


    // tok is global to all these parsing methods;
    // scan just calls the scanner's scan method and saves the result in tok.
    private Token tok; // the current token
    private void scan() {
	tok = scanner.scan();
    }
    private Stack symbolTable = new Stack();
    //Create a stack for our symbol table. Our vector lists of varables will be pushed on
    private int blockNum = 0;
    	
    private Scan scanner;
    Parser(Scan scanner) {
	this.scanner = scanner;
	scan();
	program();
	if( tok.kind != TK.EOF )
	    parse_error("junk after logical end of program");
    }

    private void program() {
	block();
    }

    private void block(){
    //Everytime a block is called, create a new vector list and push it onto the stack as our current list
    blockNum++;
    Vector variableList = new Vector();
	declaration_list();
	statement_list();
    }

    private void declaration_list() {
	// below checks whether tok is in first set of declaration.
	// here, that's easy since there's only one token kind in the set.
	// in other places, though, there might be more.
	// so, you might want to write a general function to handle that.
    	while( is(TK.DECLARE) ) {
    		declaration();
		}
    }
    private void statement_list() {
    	//statement_list leads to just statement which contains 4 things, and is looped like declaration
    	while( is(TK.ASSIGN) || is(TK.TILDE) || is(TK.ID) || is(TK.PRINT) || is(TK.DO) || is(TK.IF)){
    		statement();
    	}
    }

    private void declaration() {
    	//declaration must be @(declare) followed by an id, and as many IDs preceeded by commas
    	mustbe(TK.DECLARE);
    	if( is(TK.ID)){
    		
    	}
    	mustbe(TK.ID);
    	while( is(TK.COMMA) ) {
    		scan();
    		mustbe(TK.ID);
    	}
    }

    
    private void statement() {
    	//statement cant either be assignment, print, do, or if
    	if( is(TK.ASSIGN) || is(TK.TILDE) || is(TK.ID))
    	{
    		assignment();
    	}
    	if( is(TK.PRINT)){
    		print();
    	}
    	if( is(TK.DO)){
    		tk_do();
    	}
    	if( is(TK.IF)){
    		tk_if();
    	}

    }
    
    private void assignment(){
    	//assignment must be a ref_id followed by = (assign) and then an expression
    	ref_id();
    	mustbe(TK.ASSIGN);
    	expr();
    }
    
    private void print(){
    	//print is !(print) followed by an expression
    	mustbe(TK.PRINT);
    	expr();
    }
    
    private void tk_do(){
    	//do must be <(do) followed by a guarded comment followed by a >(enddo)
    	mustbe(TK.DO);
    	guarded_command();
    	mustbe(TK.ENDDO);
    }
    
    private void tk_if(){
    	//if must be a [(if) followed by a guarded comment, and as many guarded comments as needed preceeded by |(else if) and then followed
    	//by either just a ](end if) or a %(else) and a block, but always ending in ](end if)
    	mustbe(TK.IF);
    	guarded_command();
    	while( is(TK.ELSEIF)){
    		scan();					//scans are needed to check the next thing after the current token
    		guarded_command();
    	}
    	while( is(TK.ELSE)){
    		scan();					// ^^^
    		block();
    	}
    	mustbe(TK.ENDIF);
    }
    
    private void guarded_command(){
    	//gauded command must be an expression followed by a :(then) and followed by a block
    	expr();
    	mustbe(TK.THEN);
    	block();
    }
    
    private void ref_id(){
    	//red_id can be a ~(tilde) followed optionally by a number, but will always end in an ID
    	if( is(TK.TILDE))
    		
    	{
    		mustbe(TK.TILDE);
    		
    		if( is(TK.NUM))
    		{
    			mustbe(TK.NUM);
    		}
    	}
    	
    	mustbe(TK.ID);
    }
    
    private void expr(){
    	//expression must be a term and optionally followed by an addop, which is then followed by more terms
    	term();
    	while( is(TK.PLUS) || is(TK.MINUS)){
    		addop();
    		term();
    	}
    }
    
    private void term(){
    	//a term must be a factor followed optionally by a multop, which is then followed by more factors
    	factor();
    	while( is(TK.TIMES) || is(TK.DIVIDE)){
    		multop();
    		factor();
    	}
    }
    
    private void factor(){
    	//factors must be a ( (lparren) followed by an expression followed by a ) (rparen) OR a ref_id OR a number
    	if( is(TK.ID) || is(TK.TILDE))
    	{
    		ref_id();
    	}
    	else if( is(TK.NUM)){
    		num();
    	}
    	else{
    		mustbe(TK.LPAREN);
    		expr();
    		mustbe(TK.RPAREN);
    	}
    }
    
    private void addop(){
    	//addop must be either a +(plus) or a -(minus)
    	if( is(TK.PLUS)){
    		mustbe(TK.PLUS);
    	}
    	else mustbe(TK.MINUS);
    }
    
    private void multop(){
    	//multop must either be a *(times) or a / (divide)
    	if( is(TK.TIMES)){
    		mustbe(TK.TIMES);
    	}
    	else mustbe(TK.DIVIDE);
    }
    
    private void num(){
    	mustbe(TK.NUM);
    }
    
    // is current token what we want?
    private boolean is(TK tk) {
        return tk == tok.kind;
    }

    // ensure current token is tk and skip over it.
    private void mustbe(TK tk) {
	if( tok.kind != tk ) {
	    System.err.println( "mustbe: want " + tk + ", got " +
				    tok);
	    parse_error( "missing token (mustbe)" );
	}
	scan();
    }

    private void parse_error(String msg) {
	System.err.println( "can't parse: line "
			    + tok.lineNumber + " " + msg );
	System.exit(1);
    }
}
