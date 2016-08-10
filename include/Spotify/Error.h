/*
 * Error.h
 *
 *  Created on: 24 Feb 2014
 *      Author: neil
 */

#ifndef SPOTIFY_ERROR_H_
#define SPOTIFY_ERROR_H_

#include "fwd.h"
#include <Utility/Message.h>
#include <stdexcept>

namespace Spotify {

	inline void verify( sp_error error, std::string const& context )
	{
		if( error != SP_ERROR_OK )
		{
			Utility::Message() << " Error " << error << " " << context << " - " << sp_error_message( error )
					<< Utility::ThrowMessage< std::runtime_error >;
		}
	}

}


#endif /* ERROR_H_ */
