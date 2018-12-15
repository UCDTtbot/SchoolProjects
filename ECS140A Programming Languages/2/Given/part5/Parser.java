/* *** This file is given as part of the programming assignment. *** */
import java.util.*;

public class Parser {


    // tok is global to all these parsing methods;
    // scan just calls the scanner's scan method and saves the result in tok.
    private Token tok; // the current token
    private int cur_block = 0;
    private boolean already_displayed = false;
    
    Stack<Vector> symbol_table = new Stack<Vector>();				//Create the symbol table stack
    Vector<Token> checker = new Vector<Token>();							//Create a vector for our checker to check through our vector lists of variables

    
    
    private void scan() {
	tok = scanner.scan();
    }

    private Scan scanner;
    Parser(Scan scanner) {
	this.scanner = scanner;
	scan();
	program();
	if( tok.kind != TK.EOF )
	    parse_error("junk after logical end of program");
    }

    private void program() {
    System.out.println("#include <stdio.h>");
    System.out.println("main(){"); //C print out main
	block();
	System.out.println("}");
    }

    private void block(){  
    	cur_block++;
		declaration_list();
		statement_list();
		cur_block--;
		//if(!symbol_table.empty()){
			symbol_table.pop();

		//}
    }

    private void declaration_list() {
	// below checks whether tok is in first set of declaration.
	// here, that's easy since there's only one token kind in the set.
	// in other places, though, there might be more.
	// so, you might want to write a general function to handle that.
    	
    	boolean called = false;
    	Vector<Token> scoped_list = new Vector<Token>();
    	Vector<Token> toAdd = new Vector<Token>();
    	
    	while( is(TK.DECLARE) ) {
    		toAdd = declaration(scoped_list);
    		scoped_list.addAll(toAdd);
    		called = true;
		}
    	
    	if(!called){
    		Vector<Token> empty_set = new Vector<Token>();
    		symbol_table.push(empty_set);
    	}
    	else{
    		symbol_table.push(scoped_list);
    	}
    }
    private void statement_list() {
    	//statement_list leads to just statement which contains 4 things, and is looped like declaration
    	while( is(TK.ASSIGN) || is(TK.TILDE) || is(TK.ID) || is(TK.PRINT) || is(TK.DO) || is(TK.IF) || is(TK.FOR)){
    		statement();
    	}
    }

    private Vector<Token> declaration(Vector<Token> scoped_list) {
    	//declaration must be @(declare) followed by an id, and as many IDs preceeded by commas
    	mustbe(TK.DECLARE);
    	
    	//everytime a block is called, we create a new list for the current scope
        Token check;
        boolean already_declared = false;
        int scope_level = 0;
        
        
    	if( is(TK.ID)){	
    		//check our current scope for any redeclaration
    		if(scoped_list.size() == 0){
    			scoped_list.add(tok);
    			scope_level = cur_block - 1;
    			System.out.println("int x_" + scope_level + "_" + tok.string + ";");   
    		}
    		else{
	    		for(int i = 1; i < scoped_list.size(); i++){  //step through our list of declarations
	    			check = scoped_list.get(i);

	    			if(tok.string.equals(check.string) && !already_declared){		//if current ID is equal to an ID in the list, error
	    				System.err.println("redeclaration of variable " + tok.string);
	     				already_declared = true;
	    			}	
	    		}
	    		
        		if(!already_declared){ 
        			scoped_list.add(tok);
        			scope_level = cur_block - 1;
        			System.out.println("int x_" + scope_level + "_" + tok.string + ";"); //C, add variable declared with x_
        		}
    		}
    	}
    
    	//Algorithm in declaration should be made into a function because of code re-use
    	
    	
    	mustbe(TK.ID);
    	while( is(TK.COMMA) ) {
    		scan();
    		already_declared = false;
        	if( is(TK.ID)){	
        		//check our current scope for any redeclaration
        		for(int i = 0; i < scoped_list.size(); i++){  //step through our list of declarations
        			check = scoped_list.get(i);

        			if(tok.string.equals(check.string) && !already_declared){		//if current ID is equal to an ID in the list, error
	    				System.err.println("redeclaration of variable " + tok.string);
        				already_declared = true;
        			}	
        		}	
        		
        		if(!already_declared){ 
        			scoped_list.add(tok);
        			scope_level = cur_block - 1;
        			System.out.println("int x_" + scope_level + "_" + tok.string + ";"); //C, add variable declared with x_
        		}
				
			}
        	mustbe(TK.ID);
        }
    	return scoped_list;
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
    	if( is(TK.FOR)){
    		tk_for();
    	}

    }
    
    private void assignment(){
    	//assignment must be a ref_id followed by = (assign) and then an expression
    	ref_id();
    	mustbe(TK.ASSIGN);
    	System.out.println(" = ");
    	expr();
    	System.out.println(";");
    }
    
    private void print(){
    	//print is !(print) followed by an expression
    	mustbe(TK.PRINT);
       	System.out.print("printf(\"%" + "d\\n\"" + ",");
    	expr();
    	System.out.print(");");
    }
    
    private void tk_do(){
    	//do must be <(do) followed by a guarded comment followed by a >(enddo)
    	mustbe(TK.DO);
    	System.out.println("while(");
    	guarded_command();	//Guarded command is true if <= 0 and false if > 0
    	mustbe(TK.ENDDO);
    	System.out.println("}");
    }
    
    private void tk_for(){
    	mustbe(TK.FOR);
    	System.out.println("for(int i = ");
    	expr();
    	System.out.println("; i< ");
    	expr();
    	System.out.println("; i++){");
    	block();
    	mustbe(TK.ENDFOR);
    	System.out.println("}");
    }
    
    private void tk_if(){
    	//if must be a [(if) followed by a guarded comment, and as many guarded comments as needed preceeded by |(else if) and then followed
    	//by either just a ](end if) or a %(else) and a block, but always ending in ](end if)
    	mustbe(TK.IF);
    	System.out.println("if(");
    	guarded_command();
    	
    	while( is(TK.ELSEIF)){
    		scan();					//scans are needed to check the next thing after the current token
    		System.out.println("} else if(");
    		guarded_command();
    	}
    	while( is(TK.ELSE)){
    		scan();					// ^^^
    		System.out.println("} else{");
    		block();
    	}
    	mustbe(TK.ENDIF);
    	System.out.println("}");
    }
    
    private void guarded_command(){
    	//gauded command must be an expression followed by a :(then) and followed by a block
    	expr();
    	mustbe(TK.THEN);
    	System.out.println(" <= 0 ){");
    	block();
    }
    
    private void ref_id(){
    	//red_id can be a ~(tilde) followed optionally by a number, but will always end in an ID
    	if( is(TK.TILDE))
    	{
    		check_tilde();			//Check for tilde first, if so call tilde
    		if( is(TK.NUM)){		//If there's a number after tilde, check_tilde will return and let us do check_num
    			check_num();
    		}
    	}	
    	check_id();			//After tilde and num check for the ID
    	mustbe(TK.ID);
    	already_displayed = false;
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
    		System.out.println("(");
    		expr();
    		mustbe(TK.RPAREN);
    		System.out.println(")");
    	}
    }
    
    private void addop(){
    	//addop must be either a +(plus) or a -(minus)
    	if( is(TK.PLUS)){
    		mustbe(TK.PLUS);
    		System.out.println("+");
    	}
    	else{
    		mustbe(TK.MINUS);
    		System.out.println("-");
    	}
    }
    
    private void multop(){
    	//multop must either be a *(times) or a / (divide)
    	if( is(TK.TIMES)){
    		mustbe(TK.TIMES);
    		System.out.println("*");
    	}
    	else{
    		mustbe(TK.DIVIDE);
    		System.out.println("/");
    	}
    }
    
    private void num(){
    	if( is(TK.NUM)){
    		System.out.println(tok.string);
    	}
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
    
    private void check_id(){
    	
    	boolean found = false;
    	int spot_found = symbol_table.size();
    	int scope_level;
	    
    	if( is(TK.ID)){
	    	if(!symbol_table.empty()){
				ListIterator<Vector> stepper = symbol_table.listIterator();			//create an iterator to step throught the stack to check scopes
		    	checker = symbol_table.peek();					//Used to help check for what we need
		    	Token check;						//single check to be tested against checker
		    	
		    	//Base case of checking through the most recent scope
				for(int i = 0; i < checker.size(); i++){  //step through our list of declarations
					check = checker.get(i);
					if(tok.string.equals(check.string)){		//If token was found, declare our flag that it was found and carry on
						found = true;
						spot_found = 0; //spot found was current scope
					}
				}
				while(stepper.hasNext() && !found){			//Step through the stack if token hasn't been found
		        	checker = stepper.next();				//checker vector list is set to the one on the stack we're looking at
		        	spot_found -= 1;
		        	for(int i = 0; i < checker.size(); i++){
		        		check = checker.get(i);					//same check as above
		        		if(tok.string.equals(check.string)){
		        			found = true; 
		        		}   		
		        	}
		    	}
	    	}		
			if(!found)
			{							//If token wasn't found, throw error
				System.err.println( tok.string + " is an undeclared variable on line "
					    + tok.lineNumber);
				System.exit(1);
			}
			else if(!already_displayed){
				scope_level = symbol_table.size() - 1 - spot_found;
				System.out.println("x_" + scope_level + "_" + tok.string);
				already_displayed = true;
			}
    	}
    }
    
    public void check_tilde(){
    	boolean found = false;
    	int spot_found = 0;
    	int scope_level = 0;
    	
    	//TILDE CHECKING FOR GLOBAL
    	mustbe(TK.TILDE);

    	if( is(TK.ID)){

	    	if(!symbol_table.empty()){
		    	Token check;
		    									//Global is defined as the furthest back scope, so go to element 0
		    	checker = symbol_table.get(0);
				for(int i = 0; i < checker.size(); i++){  //step through our list of declarations
					check = checker.get(i);
					if(tok.string.equals(check.string)){		//If token found, declare flag and carry on
						found = true;
					}
				}
			}
	    	
	    	if(!found){			//If token wasn't found, throw error
	    		System.err.println("no such variable ~" + tok.string + " on line " + tok.lineNumber);
	    		System.exit(1);
	    	}
	    	else{
				System.out.println("x_" + scope_level + "_" + tok.string);
				already_displayed = true;
	    	}
    	}
    }
    
    public void check_num(){
    	boolean found = false;
    	int number_in = Integer.parseInt(tok.string);		//Parse the number in an int to use
    	int scope_level = 0;
    	mustbe(TK.NUM);

    	if( is(TK.ID)){
	    	if(!symbol_table.empty()){
		    	Token check;
		 
		    	if(number_in > symbol_table.size() - 1){  //If the scope asked to check is bigger than our stack, throw an error
		    		System.err.println("no such variable ~" + number_in + tok.string + " on line " + tok.lineNumber);
		    		System.exit(1);
		    	}
		    	else{
		    		checker = symbol_table.get(symbol_table.size() - 1 - number_in); //size - 1 - num because the way stack is, must find number in the reverse way
		    		scope_level = symbol_table.size() - 1 - number_in;
					for(int i = 0; i < checker.size() - 1; i++){  // 
						check = checker.get(i);

						if(tok.string.equals(check.string)){		//if found, declare flag and go on
							found = true;
						}
					}
		    	}
			}
	    	if(!found){ //if not found, throw an error
	    		System.err.println("no such variable ~" + number_in + tok.string + " on line " + tok.lineNumber);
	    		System.exit(1);
	    	}
	    	else{
				System.out.println("x_" + scope_level + "_" + tok.string);
				already_displayed = true;
	    	}
	    }   	
    }
}

