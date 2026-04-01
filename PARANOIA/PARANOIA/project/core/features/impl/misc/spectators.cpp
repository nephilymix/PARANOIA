#include <stdafx.hpp>

void features::misc::spectators::update() {
    std::vector<std::string> current_list;

    const auto local_pawn = systems::g_local.pawn();
    if (!local_pawn) {
        std::lock_guard<std::mutex> lock(this->m_mutex);
        this->m_spectators.clear();
        return;
    }

    // fetch all raw entities instead of using collector because collector skips dead players
    const auto entities = systems::g_entities.all();

    for (const auto& entry : entities) {
        if (entry.type != systems::entities::type::player) {
            continue;
        }

        const auto controller = entry.ptr;
        if (!controller) {
            continue;
        }

        const auto pawn_handle = g::memory.read<std::uint32_t>(
            controller + SCHEMA(ecrypt("CCSPlayerController"), "m_hPlayerPawn"_hash)
        );

        if (!pawn_handle) {
            continue;
        }

        const auto pawn = systems::g_entities.lookup(pawn_handle);

        if (!pawn || pawn == local_pawn) {
            continue;
        }

        const auto observer_services = g::memory.read<std::uintptr_t>(
            pawn + SCHEMA(ecrypt("C_BasePlayerPawn"), "m_pObserverServices"_hash)
        );

        if (!observer_services) {
            continue;
        }

        const auto observer_target_handle = g::memory.read<std::uint32_t>(
            observer_services + SCHEMA(ecrypt("CPlayer_ObserverServices"), "m_hObserverTarget"_hash)
        );

        if (!observer_target_handle) {
            continue;
        }

        const auto observer_target_pawn = systems::g_entities.lookup(observer_target_handle);

        if (observer_target_pawn == local_pawn) {
            // fetch name directly from controller
            const auto name_ptr = g::memory.read<std::uintptr_t>(
                controller + SCHEMA(ecrypt("CCSPlayerController"), "m_sSanitizedPlayerName"_hash)
            );

            if (name_ptr) {
                current_list.push_back(g::memory.read_string(name_ptr, 128));
            }
        }
    }

    std::lock_guard<std::mutex> lock(this->m_mutex);
    this->m_spectators = std::move(current_list);
}

void features::misc::spectators::render() {
    if (!settings::g_misc.m_spectators) {
        return;
    }

    std::vector<std::string> specs;
    {
        std::lock_guard<std::mutex> lock(this->m_mutex);
        specs = this->m_spectators;
    }

    static float window_x = 25.0f;
    static float window_y = 200.0f;

    float window_w = 200.0f;
    const float header_h = 35.0f;
    const float item_h = 20.0f;

    float content_h = specs.empty() ? item_h : (specs.size() * item_h);
    float window_h = header_h + content_h;

    if (window_h > 400.0f) {
        window_h = 400.0f;
    }

    if (zui::begin_window(ecrypt("spectators"), window_x, window_y, window_w, window_h, false, 150.0f, 50.0f)) {
        const auto [avail_w, avail_h] = zui::get_content_region_avail();

        if (zui::begin_nested_window(ecrypt("##specs_inner"), avail_w, avail_h)) {
            const zdraw::rgba text_color{ 255, 75, 75, 255 };
            const zdraw::rgba empty_color{ 150, 150, 150, 255 };

            if (specs.empty()) {
                zui::text_colored(ecrypt("no spectators"), empty_color);
            }
            else {
                for (const auto& name : specs) {
                    zui::text_colored(name, text_color);
                }
            }

            zui::end_nested_window();
        }
        zui::end_window();
    }
}