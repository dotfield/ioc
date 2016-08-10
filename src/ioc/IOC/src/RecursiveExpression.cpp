#include "stdafx.h"

#include <IOC/detail/RecursiveExpression.h>
#include <sstream>
#include <iostream>

namespace {
	typedef std::string::const_iterator iter_type;
	typedef std::pair< std::string, IOC::ExpressionType > TokenData;
	const TokenData tkdInit( std::string(), IOC::EError );

bool skipWhitespace( iter_type & iter, iter_type end )
{
	while( (iter != end) && isspace(*iter) )
	{
		++iter;
	}

	return iter != end;
}


}

//template class IOC_API std::vector< IOC::RecursiveExpressionPtr >;

namespace IOC {


RecursiveExpression::RecursiveExpression( RecursiveExpression * parent, std::string const& value, Type type )
	: m_value( value ), m_type( type ), m_parent( parent )
{
}

void RecursiveExpression::setError( std::string const& value )
{
	m_type = EError;
	m_value = value;
}

void RecursiveExpression::addToParent()
{
	m_parent->m_params.push_back( RecursiveExpressionPtr( this ) );
}


RecursiveExpression * RecursiveExpression::create( RecursiveExpression * parent, std::string const& value, Type type )
{
	RecursiveExpression * rec = new RecursiveExpression( parent, value, type );
	if( parent )
	{
		rec->addToParent();
	}
	return rec;
}

RecursiveExpression * RecursiveExpression::create() 
{
	return create( NULL, "Expression is empty", EError );
}



// check if it is the end of token. If it's a whitespace, skip past it but it is
// the end of token

bool isEndOfToken( std::string::const_iterator & iter, std::string::const_iterator end )
{
	if( iter==end )
	{
		return true;
	}
	else
	{
		switch( *iter )
		{
		case ' ': case '\t': 
			skipWhitespace( iter, end );
			return true;

		case ',': case '(': case ')': case ']': case ':': case '}':
			return true;

		default:
			return false;
		}
	}
}

// variable names must begin with a letter, after which they 
bool isValidVarChar( char ch ) // valid characters for variables
{
	return ::isalnum( ch ) || ch == '.' || ch == '_';
}

ExpressionType getExpressionType( std::string const& token )
{
	// special words are List, Map, Concat, Class and Library
	// they all differ on the 3rd character so use tricknology.
	// saves having to set up some lookup map for this

	if( token.size() >= 3 )
	{
		switch( token[2] )
		{
		case 'a':
			if( token == "Class" )
				return EClass;
			break;

		case 'b':
			if( token == "Library" )
				return ELibrary;
			break;

		case 'n':
			if( token == "Concat" )
				return EConcat;
			break;

		case 'r':
			if( token == "CurrentDir" )
				return ECurrentDir;
			break;

		case 's':
			if( token == "List" )
				return EList;
			break;

		default:
			break;
		}
	}

	return EObject; // this is the default
}

// like above but non-functional ones. Special words here are
// true and false which are literal bools and literal string types
// for special characters: quote, tab, newline
// once again they differ on the 3rd character
// a key difference here is that we can modify the input
ExpressionType getVariableType( std::string & token )
{
	if( token.size() >= 3 )
	{
		switch( token[2] )
		{
		case 'b':
			if( token == "tab" )
			{
				token = "\t";
				return EString;
			}
			break;

		case 'l':
			if( token == "false" )
			{
				return EBool;
			}
			break;

		case 'o':
			if( token == "quote" )
			{
				token = "\"";
				return EString;
			}
			break;

		case 'u':
			if( token == "true" )
			{
				return EBool;
			}
			break;

		case 'w':
			if( token == "newline" )
			{
				token = "\n";
				return EString;
			}
			break;

		default:
			break;
		}
	}
	return EVariable; // it's defined elsewhere
}

// We parse to in the token to the end, determining what type it is.
// If we get a parse error, this returns the text of the error in the first part and a
// type of EError. It should not throw.
TokenData readQuoted( iter_type & iter, iter_type end )
{
	TokenData res( tkdInit );

	for( ++iter; iter != end; ++iter )
	{
		if( *iter == '\"' )
		{
			res.second = EString;

			// we are not finished yet. We must move to the next token delimiter (or end)
			// which should be the next non-white character
			// and must be either , or ) if it is not end

			++iter; // move past it
			
			// This will skip past any whitespace and take us to what is next
			if( !isEndOfToken( iter, end ) )
			{
				res.first.append( " followed by unexpected character " );
				res.first.push_back( *iter );
			}
			else if( iter != end )
			{
				switch( *iter )
				{
				case ',': case ')': case ':': case '}': case ']':
					return res; // everything is fine (at least so far)
				
				case '(':
					res.first.append( " as quoted string cannot take parameter list" );
					res.second = EError;
					return res;

				default:
					res.first.append( " followed by unexpected " );
					res.first.push_back( *iter );
					res.second = EError;
					return res;
				}
			}
			return res;
		}
		else
		{
			res.first.push_back( *iter );
		}

	}
	// if we got here we have an unmatched quotes
	res.first = "Unmatched quotes";
	return res;
}

TokenData readAlpha( iter_type & iter, iter_type end )
{
	TokenData res( tkdInit );
	
	res.first.push_back( *iter );
	for( ++iter; iter != end && isValidVarChar(*iter); ++iter )
	{
		res.first.push_back( *iter );
	}
	if( ! skipWhitespace( iter, end ) )
	{
		res.second = getVariableType( res.first );
		return res;
	}
	else 
	{
		switch( *iter )
		{
		case ',': case ')': case ']': case ':': case '}':
			// there are 5 special values here
			// these two are literal booleans
			res.second = getVariableType( res.first );
			return res;

		case '(':
			res.second = getExpressionType( res.first );
			return res;

		default:
			res.second = EError;
			res.first = "Unexpected character ";
			res.first.push_back( *iter );
			return res;
		}
	}
}

TokenData readNumeric( iter_type & iter, iter_type end )
{
	TokenData res( tkdInit );
		// it's a number or an error
		// read to the next whitespace, delimiter or EOS
	bool hasPoint = false;
	for( ; !isEndOfToken(iter, end); ++iter )
	{
		res.first.push_back( *iter );
		if( *iter == '.' )
		{
			hasPoint = true;
		}
	}
	std::string extra;
	std::istringstream iss( res.first );
	if( hasPoint ) // it's a double
	{
		// yes, we could templatize these few lines
		double v;
		if( iss >> v ) 
		{
			iss >> extra;
			if( extra.empty() )
			{
				res.second = EReal;
			}
		}
	}
	else
	{
		long v;
		if( iss >> v ) 
		{
			iss >> extra;
			if( extra.empty() )
			{
				res.second = EInt;
			}
		}
	}
	
	// if we got here and didn't set to EReal or EInt
	// it wasn't a valid token
	if( res.second == EError )
	{
		// if we are still here it's not a valid number
		res.first.append ( " is not a valid token whilst reading " );
		res.first.append( iter, end );
		return res;
	}
	else
	{
		// we reached the end of our token and it's a number
		// but is our delimiter an open bracket? That would be illegal.
		// Note we will already have skipped whitespace
		if( iter != end && *iter == '(' )
		{
			res.first.append( " - unexpected (" );
			res.second = EError;
		}
	}
	return res;
}

// this is the main parsing function
TokenData readToken( std::string::const_iterator & iter,
															  std::string::const_iterator end )
{
	// create the result variable here
	std::pair< std::string, ExpressionType > res( "", EError );

	// skip past whitespace and check we have something
	if( !skipWhitespace( iter, end ) )
	{
		return std::make_pair( "Unexpected empty token or unmatched (", 
			EError );
	}

	switch( *iter )
	{
		case '\"':
		{
			return readQuoted( iter, end );

			// it's a quoted string. Read to the next quotes.
			// Anything in the quotes is part of the string and what follows is part of the string.
			// For this version we don't have any escape characters
		}
		break;

		case ')': // valid for bracketed expression with no parameters. We will check later if valid
		{
			return TokenData( std::string(), EVoid );
		}
		break;

		case '[': // start of list
		{
			return TokenData( "[", EList );
		}
		break;

		case ']': // can only happen with empty list
		{
			return TokenData( "]", EVoid );
		}
		break;

		case '}': // empty map
		{
		    return TokenData( "}", EVoid );
		}
		break;

		case '{':
		{
			return TokenData( "{", EMap );
		}
		break;
	}
	
	// alphabetic
	
	if( ::isalpha( *iter ) )
	{
		return readAlpha( iter, end );

	}
	else
	{
		// it might be numeric, or it's an error
		return readNumeric( iter, end );
	}
	

	return res;
}		


// As before we don't throw exceptions but instead return an expression with an error statement.
// This will propagate up to the entire line that has the name of the element too that is being
// defined as we don't get that in the expression.


// this is done in a class so it can be easily split into functions without having to
// either pass around too many parameters or worry about access to RecursiveExpression
// as this class is a friend.

// One instance of this class parses one "line" expression which may of course have embedded
// expressions.

// In essence it is passed an "empty" expression expr and a line of text and fills in the
// expression. If the expression has an error, it does not throw but sets the expression
// type to EError and the value to the error message.

class RecursiveExpressionParser
{

	RecursiveExpression * m_expr;
	RecursiveExpression * m_curr;
	RecursiveExpression * m_parent;

	std::string const& m_line;
	std::string const& m_currentDir;
	iter_type m_iter;
	iter_type m_end;

public:

	explicit RecursiveExpressionParser( RecursiveExpression * expr, std::string const& line, std::string const& currentDir )
		: m_expr( expr ), m_curr( expr ), m_parent( NULL ), m_line( line ), m_currentDir( currentDir ),
		m_iter( line.begin() ),	m_end( line.end() )
	{
		while( m_iter != m_end )
		{
			// first read a token
			std::pair< std::string, ExpressionType > res = readToken( m_iter, m_end );
			// If we have an error, propagate it to the head expression

			if( res.second == EError )
			{
				std::ostringstream oss;
				oss << res.first << "\n" << "  while parsing expression:\n\t" << m_line;
				m_expr->setError( oss.str() );
				return;
			}

			// if it's not a void. 
			if( res.second != EVoid )
			{
				if( m_parent )
				{
					m_curr = RecursiveExpression::create( m_parent, res.first, res.second );
				}
				else
				{
					m_expr->m_value = res.first;
					m_expr->m_type = res.second;
				}
			}
			else
			{
				if( !m_parent )
				{
					m_expr->setError( "Unexpected empty expression found" );
					return;
				}
				else
				{
					if( !m_parent->m_params.empty() )
					{
						// also an error
						m_expr->setError( "Unexpected empty expression found in " + m_parent->m_value + " in m_line " + m_line );
						return;
					}

					// otherwise it is ok for now, albeit that a Class or Library would have the wrong number of parameters,
					// as a Class must have 2 and a Library must have 1. 
				}
			}
			
			// now check what our termination is
			if( m_iter == m_end ) // we have reached the m_end. Check this is valid. We must have m_curr == m_expr
			{
				if( m_curr != m_expr )
				{
					std::ostringstream oss;
					oss << "Missing ) for " << m_parent->value() << " in expression " << m_line;
					m_expr->setError( oss.str() );
					return;
				}
				else
				{
					m_expr->m_value = res.first;
					m_expr->m_type = res.second;
				}
				// else we are at the m_end
			}
			else 
			{
				switch( *m_iter )
				{
					case '(': case '[': case '{':
					{
						// we need to "push". what is currently m_curr becomes m_parent
						m_parent = m_curr;
						m_curr = NULL; 
						++m_iter;
					}
					break;
					
					default: // either a comma, colon or a close bracket/brace/parenthesis
					{
						expressionEnd();
					}
				}
			}
		}
	}

	void expressionEnd()
	{
		// we've reached the end of the expression. This could be the end of more
		// than one, so loop if we are closing parentheses
		char terminator;

		do
		{
			terminator = *m_iter;
			if( !m_parent )
			{
				std::ostringstream oss;
				oss << "Unexpected " << terminator << " in expression " << m_line;
				m_expr->setError( oss.str() );
				return;
			}
			switch( terminator )
			{
				case ',': // token end, move to next sibling
				{
					m_curr = NULL; // will be a new sibling read in next
					++m_iter;

					if( m_parent->type() == EMap )
					{
						m_expr->setError( "A map element requires a key and " 
							+ m_line );

						return;
					}
					
					// parent type is EPair when we have read in key of the pair and are now reading the value
					if( m_parent->type() == EPair )
					{
						m_parent = m_parent->m_parent; // move back to the map
					}
					return; // we don't loop this case
				}
				case ':': // must be key element of a pair
				{
					if( m_parent->type() != EMap )
					{
						m_expr->setError( "Invalid token ':', only used for maps, in " + m_line );
						return;
					}
					else
					{
						// The current expression we were parsing is now a Pair and whatever we read
						// because the first sub-expression
					    RecursiveExpression::create( m_curr, m_curr->value(), m_curr->type() );
						m_curr->m_type = EPair; // RecursiveExpressionParser is a friend and can change this..
						m_parent = m_curr;
						m_curr = NULL;
						++m_iter; // move on to value expression for pair
					}
					return; // do not loop
				}
				case ')': // usual expression terminator
				{
					m_curr = m_parent; 
					m_parent = m_parent->m_parent;
					
					// check number of parameters here
					switch( m_curr->type() )
					{
					case ELibrary:
						if( m_curr->m_params.size() != 1 )
						{
							m_expr->setError( "A Library must have exactly one parameter (path) in " + m_line );
							return;
						}
						if( m_curr != m_expr )
						{
							m_expr->setError( "Libraries must be declared as a main expression and cannot be embedded : in " 
								+ m_line );
							return;
						}
						break;

					case EClass:
						if( m_curr->m_params.size() != 2 )
						{
							m_expr->setError( "A Class must have exactly two parameters (libraryName, symbol) in " + m_line );
							return;
						}
						if( m_curr != m_expr )
						{
							m_expr->setError( "Classes must be declared as a main expression and cannot be embedded : in " 
								+ m_line );
							return;
						}
						break;

					case EPair: case EMap: // wrong syntax for these types
						{
							m_expr->setError( "Invalid token ')' in line " + m_line );
							return;
						}
						// There are some other restrictions of pairs. The first parameter must be a literal,
						// or a variable that resolves to a literal, or Concat which resolves to literal. 
						break;

					case EList:
						{
							// A list has 2 syntaxes. If you used List( then you terminate this way.
							// If you used [ then you must terminate with ] not )
							if( m_parent->value() == "[" )
								m_expr->setError( "Invalid syntax ')' in list " + m_line );
							return;
						}
						break;

					case ECurrentDir:
						if( m_curr->m_params.size() != 0 )
						{
							m_expr->setError( "CurrentDir cannot take parameters" );
							return;
						}
						else
						{
							m_curr->m_type = EString;
							m_curr->m_value = m_currentDir;
						}
						break;

					default: // ok, it can only be EConcat or EObject
						// nothing to do, we have done what we needed to do
						// although these do have rules as to what their parameters must evaluate to, those will
						// be caught later. (e.g. none of them can take a class or library as a parameter. A Concat
						// must have only string-evaluation parameters, a Map must only have Pair parameters
						// and a List must have parameters that are all the same type )
						break;
					}
				}
				break;

				case ']': // terminator for list
				{
					if( (m_parent->type() != EList) || (m_parent->value() != "[") )
					{
						m_expr->setError( "Syntax error ']' in " + m_line );
						return;
					}
					else
					{
						m_curr = m_parent; 
						m_parent = m_parent->m_parent;
					}
				}
				break;

				case '}': // terminator for map
				{
					if( m_parent->type() == EPair ) 
					{
						m_curr = m_parent->m_parent;
						m_parent = m_curr->m_parent;
					}
					else if( (m_parent->type() == EMap) && !m_curr )
					{
						// empty map
						m_curr = m_parent; 
						m_parent = m_parent->m_parent;
					}
					else
					{
						m_expr->setError( "Syntax error '}' in " + m_line );
						return;
					}
				}
				break;

				default: // this should not occur (but can if there is an error in the expression)
				{
					std::ostringstream oss;
					oss << "Unexpected character " << terminator << " expected , or ) in " <<
						m_parent->m_value << " in expression " << m_line;

					m_expr->setError( oss.str() );
				}
			}

			// If we got here we have just had a closing token ])} which means
			// what follows next must be another closing token or it's the end
			// of the expression. Check which one it is
			
			++m_iter;
			if (! skipWhitespace( m_iter, m_end ) )
			{
				// this is what we will encounter most of the time...
				if( m_parent ) // error, it should be null
				{
					std::string open = "(";
					if( m_parent->m_value == "[" || m_parent->m_value == "{" )
					{
						open = m_parent->m_value;
					}
					m_expr->setError ( "Unmatched " + open + " for " + 
						m_parent->m_value + " in " + m_line );
				}
				// we have everything, we can just exit.
				return;
			}
		} 
		while( true ); // loop if it is the end of an expression that takes
			// parameters, thus not ) or , Do not be fooled by the fact that , ends a pair that
			// takes 2 parameters
	}
};
	
RecursiveExpressionPtr RecursiveExpression::parse( std::string const& line, std::string const& currentDir )
{

	RecursiveExpressionPtr exprShared( RecursiveExpression::create() );

	RecursiveExpressionParser parser( exprShared.get(), line, currentDir );
	return exprShared;
}
	
	// the "head" expression is expr
	// curr can move to child ones
	 // if parent is NULL we are building the main expression


	// I don't think we should ever get here.. but just in case

}
