/*
 * Config.h
 *
 *  Created on: 24 Feb 2014
 *      Author: neil
 */

#ifndef SPOTIFY_CONFIG_H_
#define SPOTIFY_CONFIG_H_

#include "fwd.h"
#include "appkey.h"
#include <thread>
#include <Utility/thread.h>
#include <chrono>
#include "Error.h"
#include <atomic>
#include <cstring>

namespace Spotify
{


struct LoginInfo
{
	std::string username;
	std::string password;
	bool	remember_me;
	std::string blob;

	LoginInfo( std::string const& u, std::string const& p, bool r ) :
		username( u ), password( p ), remember_me( r )
	{
	}

	LoginInfo() = default;
};

class Sync
{
	std::condition_variable m_condvar;
	std::mutex m_mutex;
	typedef std::unique_lock< std::mutex > lock_type;

	std::atomic_bool m_condition; // the main condition we are waiting for
	std::atomic_bool m_notify;
	sp_error m_err = SP_ERROR_OK;

	int m_nextTimeout = 0;

public:

	Sync() :
		m_condition( false ),
		m_notify( false )
	{
	}

	sp_error wait( sp_session * session );


	void notify( );

	void signal( sp_error err );
};

/**
 * Session callbacks
 */
Sync & sync();

class Config
{
	sp_session_config m_config;
	sp_session_callbacks m_callbacks;
	std::string m_cacheLocation;
	std::string m_settingsLocation;
	std::string m_userAgent;
	LoginInfo m_loginInfo;

public:
	Config( std::string const& cacheLocation,
			std::string const& settingsLocation,
			std::string const& userAgent,
			LoginInfo const& loginInfo
		  );

	sp_session_config const* get() const
	{
		return &m_config;
	}

	void login( sp_session * session ) const;
// not sure about "Callbacks" but certainly they could be configurable
};

}


#endif /* CONFIG_H_ */
