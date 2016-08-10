#include "stdafx.h"

#include <IOC/detail/RecursiveExpression.h>
#include "Utility/mapLookup.h"
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <set>
#include <fstream>

namespace {

std::string trim( std::string const& s )
{
	// there are other whitespace characters but these
	// are the only ones we care about
	const char* whites = " \t\r";

	size_t startPos = s.find_first_not_of( whites );
	if( startPos != std::string::npos )
	{
		size_t endPos = s.find_last_not_of( whites );
		return s.substr( startPos, endPos + 1 - startPos );
	}
	else
	{
		return std::string();
	}
}

//const char * const sheBangText = "^#[!].*$";
//const boost::regex sheBangRegex( sheBangText );

const char * const includeRegexText = "^#include\\s+\"([^\"]+)\"$";
const boost::regex includeRegex( includeRegexText );

const std::string wordRegexText = "[A-Za-z][A-Za-z0-9._]*";
const boost::regex wordRegex( '^' + wordRegexText + '$' );

const std::string equalsRegexText = "^(" + wordRegexText + ")\\s*=\\s*(.+)\\s*;$";
const boost::regex equalsRegex( equalsRegexText );

const char * const defineRegexText = "^#define\\s+([A-Za-z]+)\\s+\"([^\"]+)\"$";
const boost::regex defineRegex( defineRegexText );

const char * const envRegexText = "^.*\\$\\(([A-Za-z]+)\\).*$";
const boost::regex envRegex( envRegexText );

// these words are reserved and cannot be used as user-defined names
const std::string reservedWordsList[] =
{
	"Class", "Concat", "CurrentDir", "Library", "List", 
	"false", "newline", "quote", "tab", "true", "\x7f\x7f"
};

bool isReserved( std::string const& str )
{
	return std::binary_search( &reservedWordsList[0], &reservedWordsList[11], str );
}

}

namespace IOC { namespace detail {

// for a quoted string, unquote it


class ConfigLoader
{
public:
	explicit ConfigLoader( std::string const& file )
	{
		m_files.insert( file );
		loadIOCConfig( file );
	}


	std::map< std::string, RecursiveExpressionPtr > const& config() const
	{
		return m_config;
	}

	std::map< std::string, RecursiveExpressionPtr > & config()
	{
		return m_config;
	}


private:
	void loadIOCConfig( std::string const& filePath );
	bool processStatement( std::istream & is, std::string const& fileName );
	std::string expandMacros( std::string const& input, std::string const& fileName ) const;

	std::string getParentPath( std::string const& fileName ) const
	{
		std::string parentPath = boost::filesystem::path( fileName ).parent_path().string();
		if( parentPath.empty() )
		{
			parentPath = boost::filesystem::current_path().string();
		}
		parentPath.push_back('/');
		return parentPath;
	}

	std::map< std::string, RecursiveExpressionPtr > m_config;
	std::set< std::string > m_files;
	std::map< std::string, std::string > m_defines;
};


bool ConfigLoader::processStatement( std::istream & is, std::string const& fileName  )
{
	std::string line;
	std::string readLine;
	std::string parentPath = getParentPath( fileName );
	size_t lineNum = 0;
	do
	{
		// read a line. 
		if( !std::getline( is, readLine ) )
		{
			// If we are in the middle of a statement, reaching
			// end of file is an error. 
			if( !line.empty() )
			{
				std::ostringstream oss;
				oss << "End of file found in " <<
					fileName.c_str() << " Whilst processing " << line.c_str();

				throw std::invalid_argument( oss.str() );
			}
			return false; // we always terminate here
		}
		
		++lineNum;

		// trim spaces from the beginning or the end
		trim( readLine ).swap( readLine );

		// ok we could use some fancy grammar or regex.
		// anyone who is able to implement that, please go ahead
		// and do so.

		// skip blank lines and comments
		if( readLine.empty() || readLine[0] == '!' )
			continue;
		
		else if( readLine[0] == '#' )
		{
			if( !line.empty() )
			{
				std::ostringstream oss;
				oss << "Invalid " << readLine.c_str() << " line " << lineNum <<
				        " after " << line.c_str();

				throw std::invalid_argument( oss.str() );
			}

			// an include can specify a full path or a path relative to where it is
			// Note that at this level there is no concept of evaluating variables
			// or Concat() statements etc. so it must be a quoted string, so you
			// can't set a string to a directory then use a concat to get the
			// include path

			boost::smatch res;
			if( readLine.size() > 1 && readLine[1] == '!' )
//			if( boost::regex_match( readLine, res, sheBangRegex) )
			{
                // this is only allowed on the first line of the file
			    if( lineNum != 1 )
			    {
			        throw std::invalid_argument(
			                "#! (shebang) only valid on first line of the file" );
			    }

			    // otherwise continue, we do not process this line. It is there
			    // to allow UNIX systems to "execute" this file by pointing
			    // to the location of the IOCApp binary

			    // If the system is not UNIX it doesn't matter to us, it is
			    // still permitted. Similarly an invalid path is handled by
			    // the operating system, not us.

			    continue;
			}
			if( boost::regex_match( readLine, res, includeRegex ) )
			{
				std::string inc = expandMacros( res[1], fileName );
				
				// it is a full path if it contains a colon or begins with / (UNC)
				// For UNIX if it begins ~ it would also be a full path.

				if( (inc.find( ':' ) == std::string::npos) && (inc[0] != '/') )
				{
					// it's a relative path. Get the path off boost filesystem, but it seems to
					// be a bug in boost filesystem in that if the filename doesn't specify a
					// path it returns an empty string, whereas we really want . to specify
					// the current directory. (Note filesystem3 does fix this with absolute()
					// but we don't have that in this version of boost.

					inc = parentPath + inc;
				}

					// in general this will trap double inclusion but won't
					// always because in Windows paths are case-insensitive
					// so using different case you could include the same file
					// also if you put in a full path that doesn't match
					// boost's directory string eg swap / or \\  or put in
					// extra dots the std::set won't see it as a duplicate

				if( m_files.insert( inc ).second )
				{
					loadIOCConfig( inc );
				}
				return true;
			}
			else if( boost::regex_match( readLine, res, defineRegex ) )
			{
				std::string sym = res[1];
				std::string value = expandMacros( res[2], fileName );

				if( sym == "CurrentDir" )
				{
					throw std::invalid_argument( "Cannot redefine CurrentDir in " + fileName );
				}
				else
				{
					std::pair< std::map<std::string,std::string>::iterator, bool >
						res = m_defines.insert( std::make_pair( sym, value ) );

					if( !res.second )
					{
						std::ostringstream oss;
						oss << "Cannot redefine " << sym << " in " << fileName << 
							" - previously defined as " << res.first->second;
					}
				}
			}
			else
			{
				throw std::invalid_argument( "Invalid syntax " + readLine );
			}
		}
		else
		{
			line.append( readLine );
		}
		 // end of switch
	}
	while ( line.empty() || *line.rbegin() != ';' );
	
	// now we have a line find the first equals sign
	// essentially this regex should match key = value;
	// key should be a single word. value must not be empty (can be "")
	// but can otherwise be anything

	// this is a simple regex for now. (simpler than what it might be)
	// the right-hand side is .+ i.e. anything but later we will parse
	// in what is valid for this particular RHS.
	boost::smatch res;
	if( boost::regex_match( line, res, equalsRegex ) )
	{
		// this will be an error if the LHS already exists in this
		// config (but it can exist in a parent
		std::string key = res[1];
		std::string exprText = res[2];

		if( !boost::regex_match( key, res, wordRegex ) )
		{
			std::ostringstream oss;
			oss << "Valid characters in names are alpha-numeric . and _ and must begin with alphabetic";
			throw std::invalid_argument( oss.str() );
		}

		// now check it isn't reserved
		if( isReserved( key ) )
		{
			std::ostringstream oss;
			oss << key << " is a reserved word";
			throw std::invalid_argument( oss.str() );
		}

		// parse in the expression and check it parses correctly. If it doesn't,
		// it will come with an EError type.
		RecursiveExpressionPtr expr = RecursiveExpression::parse( exprText, parentPath );
		if( expr->type() == EError )
		{
			std::ostringstream oss;
			oss << expr->value() << "\n\tdefining " << key << " in file " << fileName;
			throw std::invalid_argument( oss.str() );
		}

		// Note, everything is stored in one map so you cannot use the same word to be a library,
		// a class and an object even though in theory each type is handled differently, e.g.
		// a library is only used as the first parameter in a class definition and a class is
		// only used as a parametered expression.

		std::pair<std::map<std::string, RecursiveExpressionPtr>::iterator, bool> res =
			m_config.insert( std::make_pair( key, expr ) );

		if( !res.second )
		{
			std::ostringstream oss;
			oss << "Redefinition of " << key << " previously defined to be " << expr->value();
			throw std::invalid_argument( oss.str() );
		}

		return true;
	}
	else
	{
		throw std::invalid_argument( "Invalid syntax: " + line );
	}
}


// we'll refactor as we go along
void ConfigLoader::loadIOCConfig( std::string const& filePath )
{
	// our current level of config is the one we are currently on
	boost::filesystem::path path( filePath );
	std::cerr << "Loading config file " << filePath << '\n';
	std::ifstream ifs( filePath.c_str() );
	if( !ifs )
	{
		throw std::invalid_argument( "Failed to open config file " + filePath );
	}
	else
	{
		while( processStatement( ifs, filePath ) );
	}
}

std::string ConfigLoader::expandMacros( std::string const& input, std::string const& fileName ) const
{
	std::string parsed = input;
	bool matched = true;
	do
	{
		boost::smatch res;
		if( boost::regex_match( parsed, res, envRegex ) )
		{
			std::string sym = res[1];
			std::string find = "$(" + sym + ")";

			std::string replace;
			if( sym == "CurrentDir" )
			{
				replace = getParentPath( fileName );
			}
			else if(! Utility::mapLookup( m_defines, sym, replace ) )
			{
				std::ostringstream oss;
				oss << "Undefined macro " << sym << " in " << input << " file " << fileName;
				throw std::invalid_argument( oss.str() );
			}
            std::string::size_type pos = parsed.find( find );
            parsed = parsed.substr( 0, pos ) + replace + parsed.substr( pos + sym.size() + 3 );
			matched = true;
		}
		else
		{
			matched = false;
		}
	}
	while( matched );

	return parsed;
}

void loadIOCConfigInto( str_cref filePath, std::map< std::string, RecursiveExpressionPtr > & config )
{
	ConfigLoader loader( filePath );
	config.swap( loader.config() );
}


}  }

/*
int main( int argc, char* argv[] )
{
	if( !argv[1] )
	{
		std::cerr << "Usage " << argv[0] << " Config file\n";
	}
	else
	{
		std::string configFile( argv[1] );
		try
		{
			IOC::ConfigLoader ConfigLoader( configFile );
			std::map< std::string, IOC::RecursiveExpressionPtr > const& 
		}
		catch( std::exception const& ex )
		{
			std::cerr << "ERROR: " << ex.what() << std::endl;
		}
	}
}
*/
