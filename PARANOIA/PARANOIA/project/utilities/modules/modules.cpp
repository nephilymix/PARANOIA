#include <stdafx.hpp>

bool modules::initialize( )
{
	this->client = g::memory.get_module( ecrypt("client.dll") );
	if ( !this->client )
	{
		return false;
	}

	this->engine2 = g::memory.get_module(ecrypt("engine2.dll") );
	if ( !this->engine2 )
	{
		return false;
	}

	this->tier0 = g::memory.get_module(ecrypt("tier0.dll") );
	if ( !this->tier0 )
	{
		return false;
	}

	this->schemasystem = g::memory.get_module(ecrypt("schemasystem.dll") );
	if ( !this->schemasystem )
	{
		return false;
	}

	this->vphysics2 = g::memory.get_module(ecrypt("vphysics2.dll") );
	if ( !this->vphysics2 )
	{
		return false;
	}

	g::console.print("modules initialized.");

	return true;
}