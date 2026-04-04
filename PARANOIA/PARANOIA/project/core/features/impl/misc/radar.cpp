#include <stdafx.hpp>

void features::misc::radar::set_map_data(float x, float y, float scale, ID3D11ShaderResourceView* tex)
{
    this->m_map.x = x;
    this->m_map.y = y;
    this->m_map.scale = scale;
    this->m_map.texture = tex;
}

math::vector2 features::misc::radar::world_to_radar(const math::vector3& world, const math::vector2& radar_pos, float radar_size) const
{
    // convert world position to texture pixel coordinates
    float map_px = (world.x - this->m_map.x) / this->m_map.scale;
    float map_py = (this->m_map.y - world.y) / this->m_map.scale;

    // the assets from clauadv/cs2_webradar are always 1024x1024
    float ratio = radar_size / 1024.0f;

    return math::vector2(radar_pos.x + (map_px * ratio), radar_pos.y + (map_py * ratio));
}

void features::misc::radar::on_render()
{
    if (!this->m_map.texture)
        return;

    if (!systems::g_local.valid() || !systems::g_local.alive())
        return;

    // radar ui settings
    math::vector2 radar_pos(20.0f, 20.0f);
    float radar_size = 250.0f;

    // draw background
    zdraw::rect_filled(radar_pos.x, radar_pos.y, radar_size, radar_size, zdraw::rgba(20, 20, 20, 255));

    // draw map texture
    zdraw::rect_textured(radar_pos.x, radar_pos.y, radar_size, radar_size, this->m_map.texture);

    // draw border
    zdraw::rect(radar_pos.x, radar_pos.y, radar_size, radar_size, zdraw::rgba(100, 100, 100, 255), 1.0f);

    // clip anything drawn outside radar bounds
    zdraw::push_clip_rect(radar_pos.x, radar_pos.y, radar_pos.x + radar_size, radar_pos.y + radar_size);

    const auto players = systems::g_collector.players();
    const auto local_team = systems::g_local.team();

    // draw enemies first
    for (const auto& p : players)
    {
        if (p.health <= 0 || p.team == local_team)
            continue;

        math::vector2 p_2d = this->world_to_radar(p.origin, radar_pos, radar_size);

        zdraw::circle_filled(p_2d.x, p_2d.y, 4.0f, zdraw::rgba(255, 0, 0, 255));
        zdraw::circle(p_2d.x, p_2d.y, 4.0f, zdraw::rgba(0, 0, 0, 255), 16, 1.0f);
    }

    // draw local player on top
    math::vector3 local_pos = systems::g_view.origin();
    math::vector2 local_2d = this->world_to_radar(local_pos, radar_pos, radar_size);

    zdraw::circle_filled(local_2d.x, local_2d.y, 4.5f, zdraw::rgba(0, 255, 0, 255));
    zdraw::circle(local_2d.x, local_2d.y, 4.5f, zdraw::rgba(0, 0, 0, 255), 16, 1.0f);

    // calculate and draw view direction line
    math::vector3 view_angles = systems::g_view.angles();
    float yaw_rad = math::helpers::deg_to_rad(view_angles.y);

    float end_x = local_2d.x + std::cos(yaw_rad) * 12.0f;
    float end_y = local_2d.y - std::sin(yaw_rad) * 12.0f;

    zdraw::line(local_2d.x, local_2d.y, end_x, end_y, zdraw::rgba(255, 255, 255, 255), 1.5f);

    zdraw::pop_clip_rect();
}
