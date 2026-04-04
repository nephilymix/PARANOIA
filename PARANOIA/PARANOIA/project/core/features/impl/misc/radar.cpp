#include <stdafx.hpp>

#include "resources/maps/ar_baggage.hpp"
#include "resources/maps/ar_shoots.hpp"
#include "resources/maps/ar_shoots_night.hpp"
#include "resources/maps/cs_alpine.hpp"
#include "resources/maps/cs_italy.hpp"
#include "resources/maps/cs_office.hpp"
#include "resources/maps/de_ancient.hpp"
#include "resources/maps/de_ancient_night.hpp"
#include "resources/maps/de_ancient_v1.hpp"
//#include "resources/maps/de_ancient_v2.hpp"
#include "resources/maps/de_anubis.hpp"
//#include "resources/maps/de_dust.hpp"
#include "resources/maps/de_dust2.hpp"
#include "resources/maps/de_inferno.hpp"
//#include "resources/maps/de_inferno_s2.hpp"
#include "resources/maps/de_mirage.hpp"
#include "resources/maps/de_nuke.hpp"
#include "resources/maps/de_overpass.hpp"
//#include "resources/maps/de_overpass_2v2.hpp"
#include "resources/maps/de_poseidon.hpp"
//#include "resources/maps/de_sanctum.hpp"
#include "resources/maps/de_stronghold.hpp"
#include "resources/maps/de_train.hpp"
#include "resources/maps/de_vertigo.hpp"
#include "resources/maps/de_warden.hpp"

namespace features::misc {

	std::unordered_map<std::string, radar::map_meta_t>& radar::get_map_database()
	{
		static std::unordered_map<std::string, map_meta_t> database = {
			{ "ar_baggage", { -1316.0f, 1288.0f, 2.539062f, map_ar_baggage, sizeof(map_ar_baggage) } },
			{ "ar_shoots", { -1368.0f, 1952.0f, 2.687500f, map_ar_shoots, sizeof(map_ar_shoots) } },
			{ "ar_shoots_night", { -1368.0f, 1952.0f, 2.687500f, map_ar_shoots_night, sizeof(map_ar_shoots_night) } },
			{ "cs_alpine", { -2107.1584f, 4687.119f, 6.5296063f, map_cs_alpine, sizeof(map_cs_alpine) } },
			{ "cs_italy", { -2647.0f, 2592.0f, 4.6f, map_cs_italy, sizeof(map_cs_italy) } },
			{ "cs_office", { -1838.0f, 1858.0f, 4.1f, map_cs_office, sizeof(map_cs_office) } },
			{ "de_ancient", { -2953.0f, 2164.0f, 5.0f, map_de_ancient, sizeof(map_de_ancient) } },
			{ "de_ancient_night", { -2953.0f, 2164.0f, 5.0f, map_de_ancient_night, sizeof(map_de_ancient_night) } },
			{ "de_ancient_v1", { -2953.0f, 2164.0f, 5.0f, map_de_ancient_v1, sizeof(map_de_ancient_v1) } },
			//{ "de_ancient_v2", { -2953.0f, 2164.0f, 5.0f, map_de_ancient_v2, sizeof(map_de_ancient_v2) } },
			{ "de_anubis", { -2796.0f, 3328.0f, 5.22f, map_de_anubis, sizeof(map_de_anubis) } },
			//{ "de_dust", { -2850.0f, 4073.0f, 6.0f, map_de_dust, sizeof(map_de_dust) } },
			{ "de_dust2", { -2476.0f, 3239.0f, 4.4f, map_de_dust2, sizeof(map_de_dust2) } },
			{ "de_inferno", { -2087.0f, 3870.0f, 4.9f, map_de_inferno, sizeof(map_de_inferno) } },
			//{ "de_inferno_s2", { -2087.0f, 3870.0f, 4.9f, map_de_inferno_s2, sizeof(map_de_inferno_s2) } },
			{ "de_mirage", { -3230.0f, 1713.0f, 5.0f, map_de_mirage, sizeof(map_de_mirage) } },
			{ "de_nuke", { -3453.0f, 2887.0f, 7.0f, map_de_nuke, sizeof(map_de_nuke) } },
			{ "de_overpass", { -4831.0f, 1781.0f, 5.2f, map_de_overpass, sizeof(map_de_overpass) } },
			//{ "de_overpass_2v2", { -4831.0f, 1781.0f, 5.2f, map_de_overpass_2v2, sizeof(map_de_overpass_2v2) } },
			{ "de_poseidon", { -1046.3943f, 1166.3942f, 3.0124886f, map_de_poseidon, sizeof(map_de_poseidon) } },
			//{ "de_sanctum", { -1692.2153f, 654.57465f, 3.4071784f, map_de_sanctum, sizeof(map_de_sanctum) } },
			{ "de_stronghold", { -2786.798f, 2598.281f, 2.9476247f, map_de_stronghold, sizeof(map_de_stronghold) } },
			{ "de_train", { -2308.0f, 2078.0f, 4.082077f, map_de_train, sizeof(map_de_train) } },
			{ "de_vertigo", { -3168.0f, 1762.0f, 4.0f, map_de_vertigo, sizeof(map_de_vertigo) } },
			{ "de_warden", { -3150.9443f, 2681.3335f, 5.642361f, map_de_warden, sizeof(map_de_warden) } }
		};
		return database;
	}

	void radar::update_map(const std::string& map_name)
	{
		// do not recreate texture if map is the same
		if (map_name == this->m_last_map_name)
			return;

		this->m_last_map_name = map_name;
		this->m_current_map.texture.Reset();

		auto& db = this->get_map_database();
		auto it = db.find(map_name);

		if (it != db.end())
		{
			this->m_current_map.x = it->second.x;
			this->m_current_map.y = it->second.y;
			this->m_current_map.scale = it->second.scale;

			if (it->second.image_data && it->second.image_size > 0)
			{
				std::span<const std::byte> data_span(
					reinterpret_cast<const std::byte*>(it->second.image_data),
					it->second.image_size
				);
				this->m_current_map.texture = zdraw::load_texture_from_memory(data_span);
			}
		}
	}

	math::vector2 radar::world_to_radar(const math::vector3& world, const math::vector2& radar_pos, float radar_size) const
	{
		// valid scale check to prevent division by zero
		if (this->m_current_map.scale <= 0.0f)
			return math::vector2(0.0f, 0.0f);

		float map_px = (world.x - this->m_current_map.x) / this->m_current_map.scale;
		float map_py = (this->m_current_map.y - world.y) / this->m_current_map.scale;

		// images from overview are typically 1024x1024
		float ratio = radar_size / 1024.0f;

		return math::vector2(radar_pos.x + (map_px * ratio), radar_pos.y + (map_py * ratio));
	}

	void radar::on_render()
	{
		if (!this->m_current_map.texture)
			return;

		if (!systems::g_local.valid() || !systems::g_local.alive())
			return;

		// ui configuration
		math::vector2 radar_pos(20.0f, 20.0f);
		float radar_size = 250.0f;

		// radar background and map texture
		zdraw::rect_filled(radar_pos.x, radar_pos.y, radar_size, radar_size, zdraw::rgba(20, 20, 20, 255));
		zdraw::rect_textured(radar_pos.x, radar_pos.y, radar_size, radar_size, this->m_current_map.texture.Get());
		zdraw::rect(radar_pos.x, radar_pos.y, radar_size, radar_size, zdraw::rgba(100, 100, 100, 255), 1.0f);

		// clip rendering to radar bounds
		zdraw::push_clip_rect(radar_pos.x, radar_pos.y, radar_pos.x + radar_size, radar_pos.y + radar_size);

		const auto players = systems::g_collector.players();
		const auto local_team = systems::g_local.team();

		// render enemies
		for (const auto& p : players)
		{
			if (p.health <= 0 || p.team == local_team)
				continue;

			math::vector2 p_2d = this->world_to_radar(p.origin, radar_pos, radar_size);
			zdraw::circle_filled(p_2d.x, p_2d.y, 4.0f, zdraw::rgba(255, 0, 0, 255));
			zdraw::circle(p_2d.x, p_2d.y, 4.0f, zdraw::rgba(0, 0, 0, 255), 16, 1.0f);
		}

		// render local player
		math::vector3 local_pos = systems::g_view.origin();
		math::vector2 local_2d = this->world_to_radar(local_pos, radar_pos, radar_size);

		zdraw::circle_filled(local_2d.x, local_2d.y, 4.5f, zdraw::rgba(0, 255, 0, 255));
		zdraw::circle(local_2d.x, local_2d.y, 4.5f, zdraw::rgba(0, 0, 0, 255), 16, 1.0f);

		// view direction calculation
		math::vector3 view_angles = systems::g_view.angles();
		float yaw_rad = math::helpers::deg_to_rad(view_angles.y);

		float end_x = local_2d.x + std::cos(yaw_rad) * 12.0f;
		float end_y = local_2d.y - std::sin(yaw_rad) * 12.0f;

		zdraw::line(local_2d.x, local_2d.y, end_x, end_y, zdraw::rgba(255, 255, 255, 255), 1.5f);

		zdraw::pop_clip_rect();
	}

} // namespace features::misc
