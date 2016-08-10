#ifndef UTILITY_MESSAGE_H
#define UTILITY_MESSAGE_H

#include "api.h"
#include <memory>
#include <sstream>

namespace Utility {

class UTILITY_API Message
{
private:
   std::shared_ptr< std::ostringstream > m_oss;

public:
   Message() :
      m_oss( new std::ostringstream )
   {
   }

   explicit Message( std::string const& str ) :
     m_oss( new std::ostringstream( str ) )
   {
   }

  // general case. Note it is const, it doesn't modify Message.
   template< typename T >
   const Message& operator<<(  T const& t ) const
   {
       *m_oss << t;
       return *this;
   }     

   typedef Message const& (*Manipulator)(Message const&);

   // special handler case
   const Message& operator<<( Manipulator manip ) const
   {
       return manip( *this );
   }

   std::string str() const
   {
      return m_oss->str();
   }
};

template< typename Ex > Message const& ThrowMessage( Message const& msg )
{
   throw Ex( msg.str() );
}

}

#endif

