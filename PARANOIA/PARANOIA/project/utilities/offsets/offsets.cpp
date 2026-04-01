#include <stdafx.hpp>

bool offsets::initialize( )
{
	// csgo_input
	{
		const auto initial = g::memory.find_pattern( g::modules.client, ecrypt("48 89 05 ? ? ? ? 0F 57 C0 0F 11 05") );
		if ( !initial )
		{
			g::console.print("input address fail!");
			return false;
		}

		csgo_input = g::memory.resolve_rip( initial );
		if ( !csgo_input )
		{
			g::console.print("input address rip fail!");
			return false;
		}
	}

	// entity_list
	{
//		const auto initial = g::memory.find_pattern( g::modules.client, ecrypt("48 89 35 ? ? ? ? 48 85 F6") );
		const auto initial = g::memory.find_pattern(g::modules.client, ecrypt("48 89 0D ? ? ? ? E9 ? ? ? ? CC"));
		if ( !initial )
		{
			g::console.print("entity_list address fail!");
			return false;
		}

		entity_list = g::memory.resolve_rip( initial );
		if ( !entity_list )
		{
			g::console.print("entity_list address rip fail!");
			return false;
		}
	}

	// local_player_controller
	{
		const auto initial = g::memory.find_pattern( g::modules.client, ecrypt("48 8B 05 ? ? ? ? 41 89 BE") );
		if ( !initial )
		{
			g::console.print("local_player_controller address fail!");
			return false;
		}

		local_player_controller = g::memory.resolve_rip( initial );
		if ( !local_player_controller )
		{
			g::console.print("local_player_controller address rip fail!");
			return false;
		}
	}

	// global_vars
	{
		const auto initial = g::memory.find_pattern( g::modules.client, ecrypt("48 89 15 ? ? ? ? 48 89 42") );
		if ( !initial )
		{
			g::console.print("global_vars address fail!");
			return false;
		}

		global_vars = g::memory.resolve_rip( initial );
		if ( !global_vars )
		{
			g::console.print("global_vars address rip fail!");
			return false;
		}
	}

	// view_matrix
	{
//		const auto initial = g::memory.find_pattern( g::modules.client, ecrypt("C6 86 F0 12 00 00 01 48 8D 0D ? ? ? ?") );
		const auto initial = g::memory.find_pattern(g::modules.client, ecrypt("48 8D 0D ? ? ? ? 48 C1 E0 06"));
		if ( !initial )
		{
			g::console.print("view_matrix address fail!");
			return false;
		}

		view_matrix = g::memory.resolve_rip( initial );
		if ( !view_matrix )
		{
			g::console.print("view_matrix address rip fail!");
			return false;
		}
	}
	g::console.print("offsets valid...");
	return true;
}