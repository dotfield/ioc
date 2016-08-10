/*
 * Session.h
 *
 *  Created on: 24 Feb 2014
 *      Author: neil
 */

#ifndef SPOTIFY_SESSION_H_
#define SPOTIFY_SESSION_H_

#include "fwd.h"
#include <memory>
#include <Utility/Message.h>
#include "Config.h" // can refactor later
#include "Error.h"


namespace Spotify {


class Session
{
	std::shared_ptr< sp_session > m_session;
	std::shared_ptr< Config > m_config;
public:
	explicit Session( std::shared_ptr<Config> const& config  ) : m_config( config )
	{
		sp_session * session;
		sp_error err = sp_session_create( config->get(), &session );
		verify( err, "creating spotify session ");
		m_session.reset( session, sp_session_release );

		config->login( session );
	}

	sp_session * get() const // yes it returns a non-const one, because we need it and there is no need for a const one
	{
		return m_session.get();
	}

	Sync * sync() const
	{
		return static_cast<Sync *>( sp_session_userdata( m_session.get() ) );
	}
};

}


#endif /* SESSION_H_ */
