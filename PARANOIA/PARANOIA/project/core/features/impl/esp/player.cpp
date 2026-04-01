#include <stdafx.hpp>

namespace features::esp {

	void player::on_render( )
	{
		const auto& cfg = settings::g_esp.m_player;
		if ( !cfg.enabled )
		{
			return;
		}

		const auto global_vars = g::memory.read<std::uintptr_t>( g::offsets.global_vars );
		if ( !global_vars )
		{
			return;
		}

		const auto current_time = g::memory.read<float>( global_vars + 0x30 );

		for ( const auto& player : systems::g_collector.players( ) )
		{
			if ( !systems::g_local.is_enemy( player.team ) )
			{
				continue;
			}

			const auto bones = systems::g_bones.get( player.bone_cache );
			if ( !bones.is_valid( ) )
			{
				continue;
			}

			const auto bounds = systems::g_bounds.get( bones );
			if ( !bounds.is_valid( ) )
			{
				continue;
			}

			draw_offsets offsets{};

			if ( cfg.m_box.enabled )
			{
				this->add_box( bounds, cfg.m_box, player.is_visible );
			}

			if ( cfg.m_skeleton.enabled )
			{
				this->add_skeleton( bones, cfg.m_skeleton, player.is_visible );
			}

			if (cfg.m_chams.enabled)
			{
				// Pass existing bone data - avoiding redundant memory reads!
				features::esp::g_mesh_renderer.render_player(bones, cfg.m_chams);
			}

			if ( cfg.m_hitboxes.enabled )
			{
				this->add_hitboxes( bones, player, cfg.m_hitboxes, current_time );
			}

			if ( cfg.m_health_bar.enabled )
			{
				this->add_health_bar( bounds, player, cfg.m_health_bar, offsets );
			}

			if ( cfg.m_ammo_bar.enabled && player.weapon.max_ammo > 0 )
			{
				this->add_ammo_bar( bounds, player, cfg.m_ammo_bar, offsets );
			}

			if ( cfg.m_name.enabled && !player.display_name.empty( ) )
			{
				this->add_name( bounds, player, cfg.m_name, offsets );
			}

			if ( cfg.m_weapon.enabled && !player.weapon.name.empty( ) )
			{
				this->add_weapon( bounds, player, cfg.m_weapon, offsets );
			}

			if ( cfg.m_info_flags.enabled )
			{
				this->add_flags( bounds, player, cfg.m_info_flags, offsets );
			}
		}
	}

	void player::add_box( const systems::bounds::data& bounds, const settings::esp::player::box& cfg, bool is_visible )
	{
		const auto& color = is_visible ? cfg.visible_color : cfg.occluded_color;

		const auto x = std::floorf( bounds.min.x );
		const auto y = std::floorf( bounds.min.y );
		const auto w = std::floorf( bounds.max.x - bounds.min.x );
		const auto h = std::floorf( bounds.max.y - bounds.min.y );

		if ( cfg.fill )
		{
			constexpr auto edge_alpha{ 0.5f };
			constexpr auto center_alpha{ 0.08f };
			constexpr auto center_brightness{ 0.4f };
			constexpr auto desaturation{ 0.7f };

			const auto r = color.r / 255.0f;
			const auto g = color.g / 255.0f;
			const auto b = color.b / 255.0f;
			const auto avg = ( r + g + b ) / 3.0f;

			const auto edge_r = r * desaturation + avg * ( 1.0f - desaturation );
			const auto edge_g = g * desaturation + avg * ( 1.0f - desaturation );
			const auto edge_b = b * desaturation + avg * ( 1.0f - desaturation );

			const auto edge_color = zdraw::rgba( static_cast< std::uint8_t >( edge_r * 255 ), static_cast< std::uint8_t >( edge_g * 255 ), static_cast< std::uint8_t >( edge_b * 255 ), static_cast< std::uint8_t >( 255 * edge_alpha ) );
			const auto center_color = zdraw::rgba( static_cast< std::uint8_t >( edge_r * 255 * center_brightness ), static_cast< std::uint8_t >( edge_g * 255 * center_brightness ), static_cast< std::uint8_t >( edge_b * 255 * center_brightness ), static_cast< std::uint8_t >( 255 * center_alpha ) );

			const auto mid_y = y + h * 0.5f;
			zdraw::rect_filled_multi_color( x + 1, y + 1, w - 2, mid_y - y - 1, edge_color, edge_color, center_color, center_color );
			zdraw::rect_filled_multi_color( x + 1, mid_y, w - 2, y + h - mid_y - 1, center_color, center_color, edge_color, edge_color );
		}

		if ( cfg.style == settings::esp::player::box::style0::full )
		{
			if ( cfg.outline )
			{
				zdraw::rect( x - 1, y - 1, w + 2, h + 2, zdraw::rgba( 0, 0, 0, 180 ), 1.0f );
				zdraw::rect( x, y, w, h, zdraw::rgba( 0, 0, 0, 200 ), 2.0f );
			}

			zdraw::rect( x, y, w, h, color, 1.0f );
		}
		else
		{
			const auto corner = std::min( cfg.corner_length, std::min( w, h ) * 0.4f );

			if ( cfg.outline )
			{
				zdraw::rect_cornered( x - 1, y - 1, w + 2, h + 2, zdraw::rgba( 0, 0, 0, 180 ), corner + 1, 1.0f );
				zdraw::rect_cornered( x, y, w, h, zdraw::rgba( 0, 0, 0, 200 ), corner, 2.0f );
			}

			zdraw::rect_cornered( x, y, w, h, color, corner, 1.0f );
		}
	}

	void player::add_skeleton( const systems::bones::data& bones, const settings::esp::player::skeleton& cfg, bool is_visible )
	{
		const auto& color = is_visible ? cfg.visible_color : cfg.occluded_color;

		constexpr std::array<std::pair<std::uint32_t, std::uint32_t>, 18> connections
		{ {
			{ 6, 5 },
			{ 5, 4 },
			{ 4, 3 },
			{ 3, 2 },
			{ 2, 1 },
			{ 1, 0 },
			{ 4, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 4, 13 },
			{ 13, 14 },
			{ 14, 15 },
			{ 0, 22 },
			{ 22, 23 },
			{ 23, 24 },
			{ 0, 25 },
			{ 25, 26 },
			{ 26, 27 },
		} };

		for ( const auto& [from, to] : connections )
		{
			const auto from_screen = systems::g_view.project( bones.get_position( from ) );
			const auto to_screen = systems::g_view.project( bones.get_position( to ) );

			if ( !systems::g_view.projection_valid( from_screen ) || !systems::g_view.projection_valid( to_screen ) )
			{
				continue;
			}

			zdraw::line( from_screen.x, from_screen.y, to_screen.x, to_screen.y, color, cfg.thickness );
		}
	}

	void player::add_hitboxes( const systems::bones::data& bones, const systems::collector::player& player, const settings::esp::player::hitboxes& cfg, float current_time )
	{
		const auto& color = player.is_visible ? cfg.visible_color : cfg.occluded_color;
		const auto eye_pos = systems::g_view.origin( );

		auto& anim = this->m_animations[ player.controller ];

		if ( player.health < anim.last_health )
		{
			anim.last_damage_time = current_time;
		}

		anim.last_health = player.health;

		const auto hp = std::clamp( player.health / 100.0f, 0.0f, 1.0f );
		const auto flash_t = std::clamp( 1.0f - ( current_time - anim.last_damage_time ) * 2.5f, 0.0f, 1.0f );

		std::vector<std::vector<poly2d::point>> pills;

		for ( const auto& hb : player.hitboxes )
		{
			if ( hb.index < 0 || hb.bone < 0 || hb.radius <= 0.0f )
			{
				continue;
			}

			const auto& bone = bones.bones[ hb.bone ];
			const auto center_local = ( hb.mins + hb.maxs ) * 0.5f;
			const auto center_world = bone.position + math::helpers::rotate_by_quat( bone.rotation, center_local );

			const auto half_extent = ( hb.maxs - hb.mins ) * 0.5f;
			const auto longest = std::max( { std::abs( half_extent.x ), std::abs( half_extent.y ), std::abs( half_extent.z ) } );

			math::vector3 axis_local{};
			if ( std::abs( half_extent.x ) >= std::abs( half_extent.y ) && std::abs( half_extent.x ) >= std::abs( half_extent.z ) )
			{
				axis_local = { longest, 0.0f, 0.0f };
			}
			else if ( std::abs( half_extent.y ) >= std::abs( half_extent.z ) )
			{
				axis_local = { 0.0f, longest, 0.0f };
			}
			else
			{
				axis_local = { 0.0f, 0.0f, longest };
			}

			const auto axis_world = math::helpers::rotate_by_quat( bone.rotation, axis_local );
			const auto cap_a = center_world - axis_world;
			const auto cap_b = center_world + axis_world;

			const auto sa = systems::g_view.project( cap_a );
			const auto sb = systems::g_view.project( cap_b );

			if ( !systems::g_view.projection_valid( sa ) || !systems::g_view.projection_valid( sb ) )
			{
				continue;
			}

			const auto view_dir = ( center_world - eye_pos ).normalized( );
			auto perp = axis_world.cross( view_dir );
			const auto pl = perp.length( );

			if ( pl < 0.001f )
			{
				perp = axis_world.cross( math::vector3{ 0.0f, 0.0f, 1.0f } );
				const auto pl2 = perp.length( );
				perp = pl2 > 0.001f ? perp / pl2 : math::vector3{ 1.0f, 0.0f, 0.0f };
			}
			else
			{
				perp = perp / pl;
			}

			const auto perp_world = perp * hb.radius;
			const auto p_left = systems::g_view.project( center_world + perp_world );
			const auto p_right = systems::g_view.project( center_world - perp_world );

			if ( !systems::g_view.projection_valid( p_left ) || !systems::g_view.projection_valid( p_right ) )
			{
				continue;
			}

			const auto screen_radius = std::sqrt( ( p_left.x - p_right.x ) * ( p_left.x - p_right.x ) + ( p_left.y - p_right.y ) * ( p_left.y - p_right.y ) ) * 0.5f;
			if ( screen_radius <= 0.0f || screen_radius > 800.0f )
			{
				continue;
			}

			pills.push_back( poly2d::make_pill( { sa.x, sa.y }, { sb.x, sb.y }, screen_radius, 4 ) );
		}

		if ( pills.empty( ) )
		{
			return;
		}

		auto merged = poly2d::union_pills( pills );

		for ( const auto& outline : merged.outlines )
		{
			if ( outline.size( ) < 3 )
			{
				continue;
			}

			if ( cfg.fill )
			{
				auto min_y = outline[ 0 ].y, max_y = outline[ 0 ].y;
				for ( const auto& p : outline )
				{
					min_y = std::min( min_y, p.y );
					max_y = std::max( max_y, p.y );
				}

				const auto range = max_y - min_y;

				if ( hp < 1.0f || flash_t > 0.0f )
				{
					const auto base_alpha{ 80 };
					const auto flash_alpha = static_cast< std::uint8_t >( base_alpha + ( 200 - base_alpha ) * flash_t );
					const auto fill_line = min_y + range * hp;
					const auto red = zdraw::rgba{ 220, 40, 40, flash_alpha };
					const auto tris = poly2d::triangulate( outline );

					std::vector<float> clipped;
					clipped.reserve( tris.size( ) * 2 );

					auto clip_triangle_above = [ & ]( float x0, float y0, float x1, float y1, float x2, float y2 )
						{
							const auto a0 = y0 >= fill_line;
							const auto a1 = y1 >= fill_line;
							const auto a2 = y2 >= fill_line;
							const auto count = static_cast< int >( a0 ) + static_cast< int >( a1 ) + static_cast< int >( a2 );

							if ( count == 0 )
							{
								return;
							}

							if ( count == 3 )
							{
								clipped.push_back( x0 ); clipped.push_back( y0 );
								clipped.push_back( x1 ); clipped.push_back( y1 );
								clipped.push_back( x2 ); clipped.push_back( y2 );
								return;
							}

							auto lerp_x = [ ]( float ax, float ay, float bx, float by, float cy ) { return ax + ( bx - ax ) * ( ( cy - ay ) / ( by - ay ) ); };

							if ( count == 1 )
							{
								float tx, ty, ix0, ix1;
								if ( a0 ) { tx = x0; ty = y0; ix0 = lerp_x( x0, y0, x1, y1, fill_line ); ix1 = lerp_x( x0, y0, x2, y2, fill_line ); }
								else if ( a1 ) { tx = x1; ty = y1; ix0 = lerp_x( x1, y1, x0, y0, fill_line ); ix1 = lerp_x( x1, y1, x2, y2, fill_line ); }
								else { tx = x2; ty = y2; ix0 = lerp_x( x2, y2, x0, y0, fill_line ); ix1 = lerp_x( x2, y2, x1, y1, fill_line ); }

								clipped.push_back( tx );  clipped.push_back( ty );
								clipped.push_back( ix0 ); clipped.push_back( fill_line );
								clipped.push_back( ix1 ); clipped.push_back( fill_line );
							}
							else
							{
								float bx, by, kx0, ky0, kx1, ky1;
								if ( !a0 ) { bx = x0; by = y0; kx0 = x1; ky0 = y1; kx1 = x2; ky1 = y2; }
								else if ( !a1 ) { bx = x1; by = y1; kx0 = x0; ky0 = y0; kx1 = x2; ky1 = y2; }
								else { bx = x2; by = y2; kx0 = x0; ky0 = y0; kx1 = x1; ky1 = y1; }

								const auto ix0 = lerp_x( bx, by, kx0, ky0, fill_line );
								const auto ix1 = lerp_x( bx, by, kx1, ky1, fill_line );

								clipped.push_back( kx0 ); clipped.push_back( ky0 );
								clipped.push_back( kx1 ); clipped.push_back( ky1 );
								clipped.push_back( ix0 ); clipped.push_back( fill_line );

								clipped.push_back( kx1 ); clipped.push_back( ky1 );
								clipped.push_back( ix0 ); clipped.push_back( fill_line );
								clipped.push_back( ix1 ); clipped.push_back( fill_line );
							}
						};

					for ( std::size_t i = 0; i + 5 < tris.size( ); i += 6 )
					{
						clip_triangle_above( tris[ i ], tris[ i + 1 ], tris[ i + 2 ], tris[ i + 3 ], tris[ i + 4 ], tris[ i + 5 ] );
					}

					for ( std::size_t i = 0; i + 5 < clipped.size( ); i += 6 )
					{
						zdraw::triangle_filled( clipped[ i ], clipped[ i + 1 ], clipped[ i + 2 ], clipped[ i + 3 ], clipped[ i + 4 ], clipped[ i + 5 ], red );
					}
				}
			}

			if ( cfg.outline )
			{
				std::vector<float> flat;
				flat.reserve( outline.size( ) * 2 );

				for ( const auto& p : outline )
				{
					flat.push_back( p.x );
					flat.push_back( p.y );
				}

				zdraw::polyline( std::span<const float>( flat.data( ), flat.size( ) ), color, true );
			}
		}
	}

	void player::add_health_bar( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::health_bar& cfg, draw_offsets& offsets )
	{
		auto& anim = this->m_animations[ player.controller ];

		constexpr auto bar_size{ 3.5f };
		constexpr auto padding{ 4.0f };

		const auto clamped_health = std::clamp( player.health, 0, 100 );
		const auto target_fraction = clamped_health / 100.0f;

		if ( !anim.initialized || ( target_fraction - anim.health.value( ) > 0.5f ) )
		{
			anim.health.snap( target_fraction );
			anim.initialized = true;
		}
		else
		{
			anim.health.set_target( target_fraction );
			anim.health.update( );
		}

		const auto fraction = anim.health.value( );
		const auto outline_size = cfg.outline ? 1.0f : 0.0f;
		const auto vertical = cfg.position == settings::esp::player::health_bar::position::left;

		const auto bar_w = vertical ? bar_size : std::floorf( bounds.width( ) );
		const auto bar_h = vertical ? std::floorf( bounds.height( ) ) : bar_size;
		const auto filled = std::floorf( ( vertical ? bar_h : bar_w ) * fraction );

		const auto x = [ & ]( )
			{
				if ( cfg.position == settings::esp::player::health_bar::position::left )
				{
					return std::floorf( bounds.min.x - bar_size - padding - offsets.left - outline_size );
				}

				return std::floorf( bounds.min.x );
			}( );

		const auto y = [ & ]( )
			{
				switch ( cfg.position )
				{
				case settings::esp::player::health_bar::position::left: return std::floorf( bounds.min.y );
				case settings::esp::player::health_bar::position::top: return std::floorf( bounds.min.y - bar_size - padding - offsets.top - outline_size );
				case settings::esp::player::health_bar::position::bottom: return std::floorf( bounds.max.y + padding + offsets.bottom + outline_size );
				}
				return 0.0f;
			}( );

		switch ( cfg.position )
		{
		case settings::esp::player::health_bar::position::left: offsets.left += bar_size + padding + ( outline_size * 2.0f ); break;
		case settings::esp::player::health_bar::position::top: offsets.top += bar_size + padding + ( outline_size * 2.0f ); break;
		case settings::esp::player::health_bar::position::bottom: offsets.bottom += bar_size + padding + ( outline_size * 2.0f ); break;
		}

		if ( cfg.outline )
		{
			zdraw::rect_filled( x - 1, y - 1, bar_w + 2, bar_h + 2, cfg.outline_color );
		}

		zdraw::rect_filled( x, y, bar_w, bar_h, cfg.background_color );

		if ( filled > 0 )
		{
			if ( cfg.gradient )
			{
				if ( vertical )
				{
					zdraw::rect_filled_multi_color( x, y + bar_h - filled, bar_w, filled, cfg.full_color, cfg.full_color, cfg.low_color, cfg.low_color );
				}
				else
				{
					zdraw::rect_filled_multi_color( x, y, filled, bar_h, cfg.low_color, cfg.full_color, cfg.full_color, cfg.low_color );
				}
			}
			else
			{
				if ( vertical )
				{
					zdraw::rect_filled( x, y + bar_h - filled, bar_w, filled, cfg.full_color );
				}
				else
				{
					zdraw::rect_filled( x, y, filled, bar_h, cfg.full_color );
				}
			}
		}

		if ( cfg.show_value && clamped_health < 100 )
		{
			zdraw::push_font( g::render.fonts( ).lexend_10);

			const auto text = std::to_string( clamped_health );
			const auto [text_w, text_h] = zdraw::measure_text( text );
			const auto text_x = std::floorf( x + ( bar_w * 0.5f ) - ( text_w * 0.5f ) );
			const auto text_y = vertical ? std::floorf( y + bar_h - filled - text_h - 2.0f ) : std::floorf( y - text_h - 2.0f );

			zdraw::text<zdraw::tstyles::outlined>( text_x, text_y, text, cfg.text_color );
			zdraw::pop_font( );
		}
	}

	void player::add_ammo_bar( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::ammo_bar& cfg, draw_offsets& offsets )
	{
		auto& anim = this->m_animations[ player.controller ];

		constexpr auto bar_size{ 3.5f };
		constexpr auto padding{ 4.0f };

		const auto clamped_ammo = std::clamp( player.weapon.ammo, 0, player.weapon.max_ammo );
		const auto target_fraction = static_cast< float >( clamped_ammo ) / player.weapon.max_ammo;

		if ( !anim.initialized || ( target_fraction - anim.ammo.value( ) > 0.5f ) )
		{
			anim.ammo.snap( target_fraction );
			anim.initialized = true;
		}
		else
		{
			anim.ammo.set_target( target_fraction );
			anim.ammo.update( );
		}

		const auto fraction = anim.ammo.value( );
		const auto outline_size = cfg.outline ? 1.0f : 0.0f;
		const auto vertical = cfg.position == settings::esp::player::ammo_bar::position::left;

		const auto bar_w = vertical ? bar_size : std::floorf( bounds.width( ) );
		const auto bar_h = vertical ? std::floorf( bounds.height( ) ) : bar_size;
		const auto filled = std::floorf( ( vertical ? bar_h : bar_w ) * fraction );

		const auto x = [ & ]( )
			{
				if ( cfg.position == settings::esp::player::ammo_bar::position::left )
				{
					return std::floorf( bounds.min.x - bar_size - padding - offsets.left - outline_size );
				}

				return std::floorf( bounds.min.x );
			}( );

		const auto y = [ & ]( )
			{
				switch ( cfg.position )
				{
				case settings::esp::player::ammo_bar::position::left: return std::floorf( bounds.min.y );
				case settings::esp::player::ammo_bar::position::top: return std::floorf( bounds.min.y - bar_size - padding - offsets.top - outline_size );
				case settings::esp::player::ammo_bar::position::bottom: return std::floorf( bounds.max.y + padding + offsets.bottom + outline_size );
				}
				return 0.0f;
			}( );

		switch ( cfg.position )
		{
		case settings::esp::player::ammo_bar::position::left: offsets.left += bar_size + padding + ( outline_size * 2.0f ); break;
		case settings::esp::player::ammo_bar::position::top: offsets.top += bar_size + padding + ( outline_size * 2.0f ); break;
		case settings::esp::player::ammo_bar::position::bottom: offsets.bottom += bar_size + padding + ( outline_size * 2.0f ); break;
		}

		if ( cfg.outline )
		{
			zdraw::rect_filled( x - 1, y - 1, bar_w + 2, bar_h + 2, cfg.outline_color );
		}

		zdraw::rect_filled( x, y, bar_w, bar_h, cfg.background_color );

		if ( filled > 0 )
		{
			if ( cfg.gradient )
			{
				if ( vertical )
				{
					zdraw::rect_filled_multi_color( x, y + bar_h - filled, bar_w, filled, cfg.full_color, cfg.full_color, cfg.low_color, cfg.low_color );
				}
				else
				{
					zdraw::rect_filled_multi_color( x, y, filled, bar_h, cfg.low_color, cfg.full_color, cfg.full_color, cfg.low_color );
				}
			}
			else
			{
				if ( vertical )
				{
					zdraw::rect_filled( x, y + bar_h - filled, bar_w, filled, cfg.full_color );
				}
				else
				{
					zdraw::rect_filled( x, y, filled, bar_h, cfg.full_color );
				}
			}
		}

		if ( cfg.show_value )
		{
			zdraw::push_font( g::render.fonts( ).lexend_10);

			const auto text = std::format( "{}/{}", clamped_ammo, player.weapon.max_ammo );
			const auto [text_w, text_h] = zdraw::measure_text( text );
			const auto text_x = std::floorf( x + ( bar_w * 0.5f ) - ( text_w * 0.5f ) );
			const auto text_y = vertical ? std::floorf( y + bar_h - filled - text_h - 2.0f ) : std::floorf( y + bar_h + 2.0f );

			zdraw::text<zdraw::tstyles::outlined>( text_x, text_y, text, cfg.text_color );
			zdraw::pop_font( );
		}
	}

	void player::add_name( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::name& cfg, draw_offsets& offsets )
	{
		zdraw::push_font( g::render.fonts( ).lexend_10);

		const auto [text_w, text_h] = zdraw::measure_text( player.display_name );
		const auto text_x = std::floorf( bounds.min.x + ( bounds.width( ) * 0.5f ) - ( text_w * 0.5f ) );
		const auto text_y = std::floorf( bounds.min.y - text_h - 2.0f - offsets.top );

		zdraw::text<zdraw::tstyles::outlined>( text_x, text_y, player.display_name, cfg.color );
		zdraw::pop_font( );

		offsets.top += text_h + 2.0f;
	}

	void player::add_weapon( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::weapon& cfg, draw_offsets& offsets )
	{
		zdraw::push_font( g::render.fonts( ).lexend_10 );

		auto total_height{ 0.0f };

		if ( cfg.display == settings::esp::player::weapon::display_type::icon || cfg.display == settings::esp::player::weapon::display_type::text_and_icon )
		{
			zdraw::push_font( g::render.fonts( ).weapons_15 );

			const auto icon = this->get_weapon_icon( player.weapon.name );
			const auto [icon_w, icon_h] = zdraw::measure_text( icon );
			const auto icon_x = std::floorf( bounds.min.x + ( bounds.width( ) * 0.5f ) - ( icon_w * 0.5f ) );
			const auto icon_y = std::floorf( bounds.max.y + 2.0f + offsets.bottom + total_height );

			zdraw::text<zdraw::tstyles::outlined>( icon_x, icon_y, icon, cfg.icon_color );
			zdraw::pop_font( );

			total_height += icon_h + 2.0f;
		}

		if ( cfg.display == settings::esp::player::weapon::display_type::text || cfg.display == settings::esp::player::weapon::display_type::text_and_icon )
		{
			const auto [text_w, text_h] = zdraw::measure_text( player.weapon.name );
			const auto text_x = std::floorf( bounds.min.x + ( bounds.width( ) * 0.5f ) - ( text_w * 0.5f ) );
			const auto text_y = std::floorf( bounds.max.y + 2.0f + offsets.bottom + total_height );

			zdraw::text<zdraw::tstyles::outlined>( text_x, text_y, player.weapon.name, cfg.text_color );

			total_height += text_h + 2.0f;
		}

		zdraw::pop_font( );

		offsets.bottom += total_height;
	}

	void player::add_flags( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::info_flags& cfg, draw_offsets& offsets )
	{
		zdraw::push_font( g::render.fonts( ).lexend_10);

		const auto x = std::floorf( bounds.max.x + 4.0f + offsets.right );
		auto y = std::floorf( bounds.min.y );
		auto max_w{ 0.0f };

		const auto draw_flag = [ & ]( const std::string& text, const zdraw::rgba& color )
			{
				zdraw::text<zdraw::tstyles::outlined>( x, y, text, color );
				const auto [text_w, text_h] = zdraw::measure_text( text );
				y += text_h;
				max_w = std::max( max_w, text_w );
			};

		if ( cfg.has( settings::esp::player::info_flags::flag::money ) )
		{
			draw_flag( std::format( "${}", player.money ), cfg.money_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::armor ) && player.armor > 0 )
		{
			draw_flag( player.has_helmet ? "hk" : "k", cfg.armor_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::kit ) && player.has_defuser )
		{
			draw_flag( ecrypt("kit"), cfg.kit_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::scoped ) && player.is_scoped )
		{
			draw_flag(ecrypt("zoom"), cfg.scoped_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::defusing ) && player.is_defusing )
		{
			draw_flag(ecrypt("defusing"), cfg.defusing_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::flashed ) && player.is_flashed )
		{
			draw_flag(ecrypt("flashed"), cfg.flashed_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::ping ) )
		{
			const auto ping_color = [ & ]( ) -> zdraw::rgba
				{
					if ( player.ping <= 20 ) return { 98, 217, 109, 255 };
					if ( player.ping <= 50 ) return { 230, 206, 137, 255 };
					if ( player.ping <= 70 ) return { 230, 156, 110, 255 };
					return { 222, 59, 59, 255 };
				}( );

			draw_flag( std::format( "{}ms", player.ping ), ping_color );
		}

		if ( cfg.has( settings::esp::player::info_flags::flag::distance ) )
		{
			const auto distance = systems::g_view.origin( ).distance( player.origin ) * 0.01905f;
			draw_flag( std::format( "{:.0f}m", distance ), cfg.distance_color );
		}

		zdraw::pop_font( );

		offsets.right += max_w + 4.0f;
	}

	std::string player::get_weapon_icon(const std::string& weapon_name)
	{
		// Static map initializes once. std::string safely copies the decrypted temporary buffer.
		static const std::unordered_map<std::string, std::string> icons
		{
			{ ecrypt("knife_ct"), ecrypt("]") }, { ecrypt("knife_t"), ecrypt("[") }, { ecrypt("knife"), ecrypt("]") },
			{ ecrypt("deagle"), ecrypt("A") }, { ecrypt("elite"), ecrypt("B") }, { ecrypt("fiveseven"), ecrypt("C") },
			{ ecrypt("glock"), ecrypt("D") }, { ecrypt("revolver"), ecrypt("J") }, { ecrypt("hkp2000"), ecrypt("E") },
			{ ecrypt("p250"), ecrypt("F") }, { ecrypt("usp_silencer"), ecrypt("G") }, { ecrypt("tec9"), ecrypt("H") },
			{ ecrypt("cz75a"), ecrypt("I") }, { ecrypt("mac10"), ecrypt("K") }, { ecrypt("ump45"), ecrypt("L") },
			{ ecrypt("bizon"), ecrypt("M") }, { ecrypt("mp7"), ecrypt("N") }, { ecrypt("mp9"), ecrypt("R") },
			{ ecrypt("p90"), ecrypt("O") }, { ecrypt("mp5sd"), ecrypt("N") }, { ecrypt("galilar"), ecrypt("Q") },
			{ ecrypt("famas"), ecrypt("R") }, { ecrypt("m4a1_silencer"), ecrypt("T") }, { ecrypt("m4a1"), ecrypt("S") },
			{ ecrypt("aug"), ecrypt("U") }, { ecrypt("sg556"), ecrypt("V") }, { ecrypt("ak47"), ecrypt("W") },
			{ ecrypt("g3sg1"), ecrypt("X") }, { ecrypt("scar20"), ecrypt("Y") }, { ecrypt("awp"), ecrypt("Z") },
			{ ecrypt("ssg08"), ecrypt("a") }, { ecrypt("xm1014"), ecrypt("b") }, { ecrypt("sawedoff"), ecrypt("c") },
			{ ecrypt("mag7"), ecrypt("d") }, { ecrypt("nova"), ecrypt("e") }, { ecrypt("negev"), ecrypt("f") },
			{ ecrypt("m249"), ecrypt("g") }, { ecrypt("taser"), ecrypt("h") }, { ecrypt("flashbang"), ecrypt("i") },
			{ ecrypt("hegrenade"), ecrypt("j") }, { ecrypt("smokegrenade"), ecrypt("k") }, { ecrypt("molotov"), ecrypt("l") },
			{ ecrypt("decoy"), ecrypt("m") }, { ecrypt("incgrenade"), ecrypt("n") }, { ecrypt("c4"), ecrypt("o") },
		};

		const auto it = icons.find(weapon_name);
		return it != icons.end() ? it->second : ecrypt("?");
	}

} // namespace features::esp