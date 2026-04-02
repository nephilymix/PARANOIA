#include <stdafx.hpp>
// Assuming crypter.hpp is included via stdafx or here
// #include "crypter.hpp" 

void menu::draw()
{
	if (pre_sys_GetAsyncKeyState(VK_F7) & 1)
	{
		this->m_open = !this->m_open;
	}

	if (!this->m_open)
	{
		return;
	}

	// Default compact window sizes
	static auto x{ 200.0f };
	static auto y{ 150.0f };
	static auto w{ 560.0f };
	static auto h{ 420.0f };

	zui::begin();

	// Adjusted minimum resize bounds to fit the new compact design (480x360 instead of 580x440)
	if (zui::begin_window(ecrypt("xtest##main"), this->m_x, this->m_y, this->m_w, this->m_h, true, 480.0f, 360.0f))
	{
		const auto [avail_w, avail_h] = zui::get_content_region_avail();

		if (zui::begin_nested_window(ecrypt("##inner"), avail_w, avail_h))
		{
			constexpr auto header_h{ 28.0f };
			constexpr auto padding{ 4.0f }; // Reduced outer padding

			zui::set_cursor_pos(padding, padding);
			this->draw_header(avail_w - padding * 2.0f, header_h);

			zui::set_cursor_pos(padding, padding + header_h + padding);
			this->draw_content(avail_w - padding * 2.0f, avail_h - header_h - padding * 3.0f);

			if (const auto win = zui::detail::get_current_window())
			{
				this->draw_accent_lines(win->bounds);
			}

			zui::end_nested_window();
		}

		zui::end_window();
	}

	zui::end();
}

/*
//orig
void menu::draw()
{
	if (pre_sys_GetAsyncKeyState(VK_F7) & 1)
	{
		this->m_open = !this->m_open;
	}

	if (!this->m_open)
	{
		return;
	}

	static auto x{ 200.0f };
	static auto y{ 150.0f };
	static auto w{ 680.0f };
	static auto h{ 510.0f };

	zui::begin();

	if (zui::begin_window(ecrypt("xtest##main"), this->m_x, this->m_y, this->m_w, this->m_h, true, 580.0f, 440.0f))
	{
		const auto [avail_w, avail_h] = zui::get_content_region_avail();

		if (zui::begin_nested_window(ecrypt("##inner"), avail_w, avail_h))
		{
			constexpr auto header_h{ 28.0f };
			constexpr auto padding{ 6.0f };

			zui::set_cursor_pos(padding, padding);
			this->draw_header(avail_w - padding * 2.0f, header_h);

			zui::set_cursor_pos(padding, padding + header_h + padding);
			this->draw_content(avail_w - padding * 2.0f, avail_h - header_h - padding * 3.0f);

			if (const auto win = zui::detail::get_current_window())
			{
				this->draw_accent_lines(win->bounds);
			}

			zui::end_nested_window();
		}

		zui::end_window();
	}

	zui::end();
}
*/

void menu::draw_header(float width, float height)
{
	if (!zui::begin_nested_window(ecrypt("##header"), width, height))
	{
		return;
	}

	const auto current = zui::detail::get_current_window();
	if (!current)
	{
		zui::end_nested_window();
		return;
	}

	const auto& style = zui::get_style();
	const auto dt = zdraw::get_delta_time();
	const auto bx = current->bounds.x;
	const auto by = current->bounds.y;
	const auto bw = current->bounds.w;
	const auto bh = current->bounds.h;

	zdraw::rect_filled(bx, by, bw, bh, zdraw::rgba{ 14, 14, 14, 255 });
	zdraw::rect(bx, by, bw, bh, zdraw::rgba{ 38, 38, 38, 255 });

	{
		auto [tw, th] = zdraw::measure_text(ecrypt("xtest"));
		zdraw::text(bx + 10.0f, by + (bh - th) * 0.5f, ecrypt("xtest"), style.accent);
	}

	constexpr auto tab_count = 3;
	constexpr auto tab_spacing{ 16.0f };

	struct tab_anim { float v{ 0.0f }; };
	static std::array<tab_anim, tab_count> anims{};

	auto cursor_x = bx + bw - 10.0f;

	for (int i = tab_count - 1; i >= 0; --i)
	{
		const char* tab_name = nullptr;
		tab t_val = tab::combat; // Initialize to default to avoid warnings

		// Decrypt strings locally within the loop to keep memory valid during the frame
		if (i == 0) { tab_name = ecrypt("combat"); t_val = tab::combat; }
		else if (i == 1) { tab_name = ecrypt("esp"); t_val = tab::esp; }
		else if (i == 2) { tab_name = ecrypt("misc"); t_val = tab::misc; }

		const auto is_sel = (this->m_tab == t_val);
		auto [tw, th] = zdraw::measure_text(tab_name);

		cursor_x -= tw;

		const auto tab_rect = zui::rect{ cursor_x, by, tw, bh };
		const auto hovered = zui::detail::mouse_hovered(tab_rect) && !zui::detail::paranoia_blocking_input();

		if (hovered && zui::detail::mouse_clicked())
		{
			this->m_tab = t_val;
		}

		auto& anim = anims[i];
		anim.v += ((is_sel ? 1.0f : 0.0f) - anim.v) * std::min(10.0f * dt, 1.0f);

		const auto text_y = by + (bh - th) * 0.5f;
		const auto col = is_sel ? zui::lighten(style.accent, 1.0f + 0.1f * anim.v) : zui::lerp(zdraw::rgba{ 110, 110, 110, 255 }, style.text, hovered ? 1.0f : 0.0f);

		zdraw::text(cursor_x, text_y, tab_name, col);

		cursor_x -= tab_spacing;
	}

	zui::end_nested_window();
}
/*

void menu::draw_header(float width, float height)
{
	if (!zui::begin_nested_window(ecrypt("##header"), width, height))
	{
		return;
	}

	const auto current = zui::detail::get_current_window();
	if (!current)
	{
		zui::end_nested_window();
		return;
	}

	const auto& style = zui::get_style();
	const auto dt = zdraw::get_delta_time();
	const auto bx = current->bounds.x;
	const auto by = current->bounds.y;
	const auto bw = current->bounds.w;
	const auto bh = current->bounds.h;

	zdraw::rect_filled(bx, by, bw, bh, zdraw::rgba{ 14, 14, 14, 255 });
	zdraw::rect(bx, by, bw, bh, zdraw::rgba{ 38, 38, 38, 255 });

	{
		auto [tw, th] = zdraw::measure_text(ecrypt("xtest"));
		zdraw::text(bx + 10.0f, by + (bh - th) * 0.5f, ecrypt("xtest"), style.accent);
	}

	// Changed static
	std::pair<const char*, tab> tabs[] = {
		{ ecrypt("combat"), tab::combat },
		{ ecrypt("esp"), tab::esp },
		{ ecrypt("misc"), tab::misc }
	};

	constexpr auto tab_count = static_cast<int>(std::size(tabs));
	constexpr auto tab_spacing{ 16.0f };

	struct tab_anim { float v{ 0.0f }; };
	static std::array<tab_anim, tab_count> anims{};

	auto cursor_x = bx + bw - 10.0f;

	for (int i = tab_count - 1; i >= 0; --i)
	{
		const auto& t = tabs[i];
		const auto is_sel = (this->m_tab == t.second);
		auto [tw, th] = zdraw::measure_text(t.first);

		cursor_x -= tw;

		const auto tab_rect = zui::rect{ cursor_x, by, tw, bh };
		const auto hovered = zui::detail::mouse_hovered(tab_rect) && !zui::detail::paranoia_blocking_input();

		if (hovered && zui::detail::mouse_clicked())
		{
			this->m_tab = t.second;
		}

		auto& anim = anims[i];
		anim.v += ((is_sel ? 1.0f : 0.0f) - anim.v) * std::min(10.0f * dt, 1.0f);

		const auto text_y = by + (bh - th) * 0.5f;
		const auto col = is_sel ? zui::lighten(style.accent, 1.0f + 0.1f * anim.v) : zui::lerp(zdraw::rgba{ 110, 110, 110, 255 }, style.text, hovered ? 1.0f : 0.0f);

		zdraw::text(cursor_x, text_y, t.first, col);

		cursor_x -= tab_spacing;
	}

	zui::end_nested_window();
}
*/

void menu::draw_content(float width, float height)
{
	// Reduced inner item paddings for a more compact look
	zui::push_style_var(zui::style_var::window_padding_x, 8.0f);
	zui::push_style_var(zui::style_var::window_padding_y, 8.0f);
	zui::push_style_var(zui::style_var::item_spacing_y, 6.0f);

	if (!zui::begin_nested_window(ecrypt("##content"), width, height))
	{
		zui::pop_style_var(3);
		return;
	}

	if (const auto win = zui::detail::get_current_window())
	{
		this->draw_accent_lines(win->bounds);
	}

	switch (this->m_tab)
	{
	case tab::combat:
		this->draw_combat();
		break;
	case tab::esp:
		this->draw_esp();
		break;
	case tab::misc:
		this->draw_misc();
		break;
	default:
		break;
	}

	zui::pop_style_var(3);
	zui::end_nested_window();
}

/*
//orig

void menu::draw_content(float width, float height)
{
	zui::push_style_var(zui::style_var::window_padding_x, 10.0f);
	zui::push_style_var(zui::style_var::window_padding_y, 10.0f);

	if (!zui::begin_nested_window(ecrypt("##content"), width, height))
	{
		zui::pop_style_var(2);
		return;
	}

	if (const auto win = zui::detail::get_current_window())
	{
		this->draw_accent_lines(win->bounds);
	}

	switch (this->m_tab)
	{
	case tab::combat:
		this->draw_combat();
		break;
	case tab::esp:
		this->draw_esp();
		break;
	case tab::misc:
		this->draw_misc();
		break;
	default:
		break;
	}

	zui::pop_style_var(2);
	zui::end_nested_window();
}
*/

void menu::draw_accent_lines(const zui::rect& bounds, float fade_ratio)
{
	const auto ix = bounds.x + 1.0f;
	const auto iw = bounds.w - 2.0f;
	const auto top_y = bounds.y + 1.0f;
	const auto bot_y = bounds.y + bounds.h - 2.0f;
	const auto accent = zui::get_accent_color();
	const auto trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
	const auto fade_w = iw * fade_ratio;
	const auto solid_w = iw - fade_w * 2.0f;

	for (const auto ly : { top_y, bot_y })
	{
		zdraw::rect_filled_multi_color(ix, ly, fade_w, 1.0f, trans, accent, accent, trans);
		zdraw::rect_filled(ix + fade_w, ly, solid_w, 1.0f, accent);
		zdraw::rect_filled_multi_color(ix + fade_w + solid_w, ly, fade_w, 1.0f, accent, trans, trans, accent);
	}
}

void menu::draw_combat()
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail();
	const auto col_w = (avail_w - 8.0f) * 0.5f;
	const auto& style = zui::get_style();

	{
		const auto win = zui::detail::get_current_window();
		if (win)
		{
			constexpr auto group_spacing{ 12.0f };
			constexpr auto bar_h{ 22.0f };
			auto gx = win->bounds.x + style.window_padding_x;
			const auto gy = win->bounds.y + win->cursor_y;

			/*
						for (int i = 0; i < 6; ++i)
						{
							auto [tw, th] = zdraw::measure_text(k_weapon_groups[i]);
							const auto gr = zui::rect{ gx, gy, tw, bar_h };
							const auto hov = zui::detail::mouse_hovered(gr) && !zui::detail::paranoia_blocking_input();

							if (hov && zui::detail::mouse_clicked())
							{
								this->m_weapon_group = i;
							}

							const auto sel = (this->m_weapon_group == i);
							const auto col = sel ? zui::get_accent_color() : zui::lerp(zdraw::rgba{ 100, 100, 100, 255 }, style.text, hov ? 1.0f : 0.0f);

							zdraw::text(gx, gy + (bar_h - th) * 0.5f, k_weapon_groups[i], col);

							if (sel)
							{
								const auto accent = zui::get_accent_color();
								const auto trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
								const auto fade = tw * 0.3f;
								zdraw::rect_filled_multi_color(gx, gy + bar_h - 2.0f, fade, 1.0f, trans, accent, accent, trans);
								zdraw::rect_filled(gx + fade, gy + bar_h - 2.0f, tw - fade * 2.0f, 1.0f, accent);
								zdraw::rect_filled_multi_color(gx + tw - fade, gy + bar_h - 2.0f, fade, 1.0f, accent, trans, trans, accent);
							}

							gx += tw + group_spacing;
						}
			*/

			for (int i = 0; i < 6; ++i)
			{
				// Lambda function to handle the drawing logic without code duplication
				// The temporary xorstr object lives only during this function call
				auto process_tab = [&](const char* group_name)
					{
						auto [tw, th] = zdraw::measure_text(group_name);
						const auto gr = zui::rect{ gx, gy, tw, bar_h };
						const auto hov = zui::detail::mouse_hovered(gr) && !zui::detail::paranoia_blocking_input();

						if (hov && zui::detail::mouse_clicked())
						{
							this->m_weapon_group = i;
						}

						const auto sel = (this->m_weapon_group == i);
						const auto col = sel ? zui::get_accent_color() : zui::lerp(zdraw::rgba{ 100, 100, 100, 255 }, style.text, hov ? 1.0f : 0.0f);

						zdraw::text(gx, gy + (bar_h - th) * 0.5f, group_name, col);

						if (sel)
						{
							const auto accent = zui::get_accent_color();
							const auto trans = zdraw::rgba{ accent.r, accent.g, accent.b, 0 };
							const auto fade = tw * 0.3f;
							zdraw::rect_filled_multi_color(gx, gy + bar_h - 2.0f, fade, 1.0f, trans, accent, accent, trans);
							zdraw::rect_filled(gx + fade, gy + bar_h - 2.0f, tw - fade * 2.0f, 1.0f, accent);
							zdraw::rect_filled_multi_color(gx + tw - fade, gy + bar_h - 2.0f, fade, 1.0f, accent, trans, trans, accent);
						}

						gx += tw + group_spacing;
					};

				// Decrypt on the fly and pass immediately to the lambda
				// Memory is wiped right after the function returns
				switch (i)
				{
				case 0: process_tab(ecrypt("pistol")); break;
				case 1: process_tab(ecrypt("smg")); break;
				case 2: process_tab(ecrypt("rifle")); break;
				case 3: process_tab(ecrypt("shotgun")); break;
				case 4: process_tab(ecrypt("sniper")); break;
				case 5: process_tab(ecrypt("lmg")); break;
				}
			}

			win->cursor_y += bar_h + style.item_spacing_y;
			win->line_height = 0.0f;
		}
	}

	auto& cfg = settings::g_combat.groups[this->m_weapon_group];

	if (zui::begin_group_box(ecrypt("aimbot"), col_w))
	{
		zui::checkbox(ecrypt("enabled##ab"), cfg.aimbot.enabled);
		zui::keybind(ecrypt("key##ab"), cfg.aimbot.key);
		zui::slider_int(ecrypt("fov##ab"), cfg.aimbot.fov, 1, 45);
		zui::slider_int(ecrypt("smoothing##ab"), cfg.aimbot.smoothing, 0, 50);
		zui::checkbox(ecrypt("head only##ab"), cfg.aimbot.head_only);
		zui::checkbox(ecrypt("visible only##ab"), cfg.aimbot.visible_only);

		// 1. store decrypted strings in static variables so they stay in memory forever
		static std::string s_hitbox = ecrypt("hitbox");
		static std::string s_nearest = ecrypt("nearest");

		// 2. create a classic c-style array of const char* for zui::combo
		const char* aim_types[]{ s_hitbox.c_str(), s_nearest.c_str() };

		// 3. call combo WITHOUT the 'if' statement, pass the array and the count (2)
		zui::combo(ecrypt("type##ab"), cfg.aimbot.type, aim_types, 2);

		// the rest of the ui will now render properly instead of disappearing
		if (cfg.aimbot.visible_only)
		{
			zui::checkbox(ecrypt("autowall##ab"), cfg.aimbot.autowall);

			if (cfg.aimbot.autowall)
			{
				zui::slider_float(ecrypt("min damage##ab"), cfg.aimbot.min_damage, 1.0f, 100.0f, ecrypt("%.0f"));
			}
		}

		zui::checkbox(ecrypt("predictive##ab"), cfg.aimbot.predictive);
		zui::checkbox(ecrypt("recoil control##ab"), cfg.aimbot.rcs);

		if (cfg.aimbot.rcs)
		{
			zui::slider_float(ecrypt("rcs pitch##ab"), cfg.aimbot.rcs_scale_x, 0.0f, 2.0f, ecrypt("%.2f"));
			zui::slider_float(ecrypt("rcs yaw##ab"), cfg.aimbot.rcs_scale_y, 0.0f, 2.0f, ecrypt("%.2f"));
		}

		zui::separator();
		zui::checkbox(ecrypt("draw fov##ab"), cfg.aimbot.draw_fov);

		if (cfg.aimbot.draw_fov)
		{
			zui::color_picker(ecrypt("fov color##ab"), cfg.aimbot.fov_color);
		}

		zui::end_group_box();
	}

	/*
	
	if (zui::begin_group_box(ecrypt("aimbot"), col_w))
	{
		zui::checkbox(ecrypt("enabled##ab"), cfg.aimbot.enabled);
		zui::keybind(ecrypt("key##ab"), cfg.aimbot.key);
		zui::slider_int(ecrypt("fov##ab"), cfg.aimbot.fov, 1, 45);
		zui::slider_int(ecrypt("smoothing##ab"), cfg.aimbot.smoothing, 0, 50);
		zui::checkbox(ecrypt("head only##ab"), cfg.aimbot.head_only);
		zui::checkbox(ecrypt("visible only##ab"), cfg.aimbot.visible_only);

		//static const char* aim_types[]{ ecrypt("hitbox"), ecrypt("nearest") };
		// Fix: Removed 'static' to prevent pointer corruption after decryption
		const char* aim_types[]{ ecrypt("hitbox"), ecrypt("nearest") };
		if (zui::combo(ecrypt("type##ab"), cfg.aimbot.type, aim_types, 2))
		{
			if (cfg.aimbot.visible_only)
			{
				zui::checkbox(ecrypt("autowall##ab"), cfg.aimbot.autowall);

				if (cfg.aimbot.autowall)
				{
					zui::slider_float(ecrypt("min damage##ab"), cfg.aimbot.min_damage, 1.0f, 100.0f, ecrypt("%.0f"));
				}
			}

			zui::checkbox(ecrypt("predictive##ab"), cfg.aimbot.predictive);
			zui::checkbox(ecrypt("recoil control##ab"), cfg.aimbot.rcs);
			if (cfg.aimbot.rcs)
			{
				zui::slider_float(ecrypt("rcs pitch##ab"), cfg.aimbot.rcs_scale_x, 0.0f, 2.0f, ecrypt("%.2f"));
				zui::slider_float(ecrypt("rcs yaw##ab"), cfg.aimbot.rcs_scale_y, 0.0f, 2.0f, ecrypt("%.2f"));
			}
			zui::separator();
			zui::checkbox(ecrypt("draw fov##ab"), cfg.aimbot.draw_fov);

			if (cfg.aimbot.draw_fov)
			{
				zui::color_picker(ecrypt("fov color##ab"), cfg.aimbot.fov_color);
			}
		}


		zui::end_group_box();
	}
	*/

	zui::same_line();

	if (zui::begin_group_box(ecrypt("triggerbot"), col_w))
	{
		zui::checkbox(ecrypt("enabled##tb"), cfg.triggerbot.enabled);
		zui::keybind(ecrypt("key##tb"), cfg.triggerbot.key);
		zui::slider_float(ecrypt("hitchance##tb"), cfg.triggerbot.hitchance, 0.0f, 100.0f, ecrypt("%.0f%%"));
		zui::slider_int(ecrypt("delay (ms)##tb"), cfg.triggerbot.delay, 0, 500);
		zui::checkbox(ecrypt("autowall##tb"), cfg.triggerbot.autowall);

		if (cfg.triggerbot.autowall)
		{
			zui::slider_float(ecrypt("min damage##tb"), cfg.triggerbot.min_damage, 1.0f, 100.0f, ecrypt("%.0f"));
		}

		zui::checkbox(ecrypt("autostop##tb"), cfg.triggerbot.autostop);

		if (cfg.triggerbot.autostop)
		{
			zui::checkbox(ecrypt("early autostop##tb"), cfg.triggerbot.early_autostop);
		}

		zui::checkbox(ecrypt("predictive##tb"), cfg.triggerbot.predictive);
		zui::end_group_box();
	}
}

void menu::draw_esp()
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail();
	const auto col_w = (avail_w - 8.0f) * 0.5f;
	auto& p = settings::g_esp.m_player;

	if (zui::begin_group_box(ecrypt("box"), col_w))
	{
		zui::checkbox(ecrypt("enabled##bx"), p.m_box.enabled);


		// Fix: Removed 'static'  static const char* box_styles[]{ ecrypt("full"), ecrypt("cornered") };
		const char* box_styles[]{ ecrypt("full"), ecrypt("cornered") };
		auto bs = static_cast<int>(p.m_box.style);

		if (zui::combo(ecrypt("style##bx"), bs, box_styles, 2))
		{
			p.m_box.style = static_cast<settings::esp::player::box::style0>(bs);
		}

		zui::checkbox(ecrypt("fill##bx"), p.m_box.fill);
		zui::checkbox(ecrypt("outline##bx"), p.m_box.outline);

		if (p.m_box.style == settings::esp::player::box::style0::cornered)
		{
			zui::slider_float(ecrypt("corner len##bx"), p.m_box.corner_length, 4.0f, 30.0f, ecrypt("%.0f"));
		}

		zui::color_picker(ecrypt("visible##bx"), p.m_box.visible_color);
		zui::color_picker(ecrypt("occluded##bx"), p.m_box.occluded_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("skeleton"), col_w))
	{
		zui::checkbox(ecrypt("enabled##sk"), p.m_skeleton.enabled);
		zui::slider_float(ecrypt("thickness##sk"), p.m_skeleton.thickness, 0.5f, 4.0f, ecrypt("%.1f"));
		zui::color_picker(ecrypt("visible##sk"), p.m_skeleton.visible_color);
		zui::color_picker(ecrypt("occluded##sk"), p.m_skeleton.occluded_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("chams"), col_w))
	{
		auto& ch = p.m_chams;
		zui::checkbox(ecrypt("enabled##chm"), ch.enabled);
		zui::checkbox(ecrypt("wireframe##chm"), ch.wireframe);

		const char* mat_types[]{ ecrypt("flat"), ecrypt("shaded"), ecrypt("glow") };
		int mat = ch.material_type;
		if (zui::combo(ecrypt("material##chm"), mat, mat_types, 3))
		{
			ch.material_type = mat;
		}

		zui::slider_float(ecrypt("alpha##chm"), ch.alpha, 0.1f, 1.0f, ecrypt("%.2f"));
		zui::color_picker(ecrypt("fill color##chm"), ch.fill_color);

		if (ch.wireframe)
		{
			zui::color_picker(ecrypt("wire color##chm"), ch.wire_color);
		}
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("hitboxes"), col_w))
	{
		auto& hb = p.m_hitboxes;
		zui::checkbox(ecrypt("enabled##hb"), hb.enabled);
		zui::checkbox(ecrypt("fill##hb"), hb.fill);
		zui::checkbox(ecrypt("outline##hb"), hb.outline);
		zui::color_picker(ecrypt("visible##hb"), hb.visible_color);
		zui::color_picker(ecrypt("occluded##hb"), hb.occluded_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("health bar"), col_w))
	{
		zui::checkbox(ecrypt("enabled##hb"), p.m_health_bar.enabled);
		zui::checkbox(ecrypt("outline##hb"), p.m_health_bar.outline);
		zui::checkbox(ecrypt("gradient##hb"), p.m_health_bar.gradient);
		zui::checkbox(ecrypt("show value##hb"), p.m_health_bar.show_value);
		zui::color_picker(ecrypt("full##hb"), p.m_health_bar.full_color);
		zui::color_picker(ecrypt("low##hb"), p.m_health_bar.low_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("items"), col_w))
	{
		auto& it = settings::g_esp.m_item;
		zui::checkbox(ecrypt("enabled##it"), it.enabled);
		zui::slider_float(ecrypt("max dist##it"), it.max_distance, 5.0f, 150.0f, ecrypt("%.0fm"));
		zui::checkbox(ecrypt("icon##it"), it.m_icon.enabled);
		zui::color_picker(ecrypt("icon color##it"), it.m_icon.color);
		zui::checkbox(ecrypt("name##it"), it.m_name.enabled);
		zui::color_picker(ecrypt("name color##it"), it.m_name.color);
		zui::checkbox(ecrypt("ammo##it"), it.m_ammo.enabled);
		zui::color_picker(ecrypt("ammo color##it"), it.m_ammo.color);
		zui::color_picker(ecrypt("empty color##it"), it.m_ammo.empty_color);
		zui::end_group_box();
	}

	const auto [cx, cy] = zui::get_cursor_pos();
	zui::set_cursor_pos(col_w + 8.0f + zui::get_style().window_padding_x, zui::get_style().window_padding_y);

	if (zui::begin_group_box(ecrypt("name / weapon"), col_w))
	{
		zui::checkbox(ecrypt("name##nm"), p.m_name.enabled);
		zui::color_picker(ecrypt("name color##nm"), p.m_name.color);
		zui::separator();
		zui::checkbox(ecrypt("weapon##wp"), p.m_weapon.enabled);

		//static const char* disp_types[]{ ecrypt("text"), ecrypt("icon"), ecrypt("text + icon") };
		const char* disp_types[]{ ecrypt("text"), ecrypt("icon"), ecrypt("text + icon") };
		auto dt = static_cast<int>(p.m_weapon.display);

		if (zui::combo(ecrypt("display##wp"), dt, disp_types, 3))
		{
			p.m_weapon.display = static_cast<settings::esp::player::weapon::display_type>(dt);
		}

		zui::color_picker(ecrypt("text color##wp"), p.m_weapon.text_color);
		zui::color_picker(ecrypt("icon color##wp"), p.m_weapon.icon_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("ammo bar"), col_w))
	{
		zui::checkbox(ecrypt("enabled##amb"), p.m_ammo_bar.enabled);
		zui::checkbox(ecrypt("outline##amb"), p.m_ammo_bar.outline);
		zui::checkbox(ecrypt("gradient##amb"), p.m_ammo_bar.gradient);
		zui::checkbox(ecrypt("show value##amb"), p.m_ammo_bar.show_value);
		zui::color_picker(ecrypt("full##amb"), p.m_ammo_bar.full_color);
		zui::color_picker(ecrypt("low##amb"), p.m_ammo_bar.low_color);
		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("projectiles"), col_w))
	{
		auto& pr = settings::g_esp.m_projectile;
		zui::checkbox(ecrypt("enabled##pr"), pr.enabled);
		zui::checkbox(ecrypt("icon##pr"), pr.show_icon);
		zui::checkbox(ecrypt("name##pr"), pr.show_name);
		zui::checkbox(ecrypt("timer bar##pr"), pr.show_timer_bar);
		zui::checkbox(ecrypt("inferno bounds##pr"), pr.show_inferno_bounds);
		zui::separator();
		zui::color_picker(ecrypt("he##pr"), pr.color_he);
		zui::color_picker(ecrypt("flash##pr"), pr.color_flash);
		zui::color_picker(ecrypt("smoke##pr"), pr.color_smoke);
		zui::color_picker(ecrypt("molotov##pr"), pr.color_molotov);
		zui::color_picker(ecrypt("decoy##pr"), pr.color_decoy);
		zui::end_group_box();
	}
	if (zui::begin_group_box(ecrypt("info flags"), col_w))
	{
		auto& fl = p.m_info_flags;
		zui::checkbox(ecrypt("enabled##flg"), fl.enabled);
		zui::separator();

		// Helper lambda to manage bitwise flags with standard checkboxes
		auto flag_checkbox = [&](const char* label, std::uint8_t flag_val, zdraw::rgba& clr) {
			bool active = fl.has(static_cast<settings::esp::player::info_flags::flag>(flag_val));
			if (zui::checkbox(label, active)) {
				if (active) fl.flags |= flag_val;
				else fl.flags &= ~flag_val;
			}
			zui::color_picker(label, clr); // Assuming color picker label maps to the same line/item
			};

		flag_checkbox(ecrypt("money##flg"), settings::esp::player::info_flags::money, fl.money_color);
		flag_checkbox(ecrypt("armor##flg"), settings::esp::player::info_flags::armor, fl.armor_color);
		flag_checkbox(ecrypt("kit##flg"), settings::esp::player::info_flags::kit, fl.kit_color);
		flag_checkbox(ecrypt("scoped##flg"), settings::esp::player::info_flags::scoped, fl.scoped_color);
		flag_checkbox(ecrypt("defusing##flg"), settings::esp::player::info_flags::defusing, fl.defusing_color);
		flag_checkbox(ecrypt("flashed##flg"), settings::esp::player::info_flags::flashed, fl.flashed_color);

		zui::end_group_box();
	}

	if (zui::begin_group_box(ecrypt("item filters"), col_w))
	{
		auto& f = settings::g_esp.m_item.m_filters;
		zui::checkbox(ecrypt("rifles##f"), f.rifles);
		zui::checkbox(ecrypt("smgs##f"), f.smgs);
		zui::checkbox(ecrypt("shotguns##f"), f.shotguns);
		zui::checkbox(ecrypt("snipers##f"), f.snipers);
		zui::checkbox(ecrypt("pistols##f"), f.pistols);
		zui::checkbox(ecrypt("heavy##f"), f.heavy);
		zui::checkbox(ecrypt("grenades##f"), f.grenades);
		zui::checkbox(ecrypt("utility##f"), f.utility);
		zui::end_group_box();
	}
}

void menu::draw_misc()
{
	const auto [avail_w, avail_h] = zui::get_content_region_avail();
	const auto col_w = (avail_w - 8.0f) * 0.5f;
	auto& gr = settings::g_misc.m_grenades;

	zui::checkbox(ecrypt("spectators list##misc"), settings::g_misc.m_spectators);

	if (zui::begin_group_box(ecrypt("grenade prediction"), col_w))
	{
		zui::checkbox(ecrypt("enabled##gr"), gr.enabled);
		zui::checkbox(ecrypt("local only##gr"), gr.local_only);
		zui::slider_float(ecrypt("line thickness##gr"), gr.line_thickness, 0.5f, 5.0f, ecrypt("%.1f"));
		zui::checkbox(ecrypt("gradient line##gr"), gr.line_gradient);
		zui::color_picker(ecrypt("line color##gr"), gr.line_color);
		zui::separator();
		zui::checkbox(ecrypt("show bounces##gr"), gr.show_bounces);
		zui::color_picker(ecrypt("bounce color##gr"), gr.bounce_color);
		zui::slider_float(ecrypt("bounce size##gr"), gr.bounce_size, 1.0f, 8.0f, ecrypt("%.1f"));
		zui::separator();
		zui::color_picker(ecrypt("detonate color##gr"), gr.detonate_color);
		zui::slider_float(ecrypt("detonate size##gr"), gr.detonate_size, 1.0f, 10.0f, ecrypt("%.1f"));
		zui::slider_float(ecrypt("fade duration##gr"), gr.fade_duration, 0.0f, 2.0f, ecrypt("%.2f"));
		zui::end_group_box();
	}

	zui::same_line();

	if (zui::begin_group_box(ecrypt("per type colors"), col_w))
	{
		zui::checkbox(ecrypt("enabled##ptc"), gr.per_type_colors);
		if (gr.per_type_colors)
		{
			zui::color_picker(ecrypt("he##ptc"), gr.color_he);
			zui::color_picker(ecrypt("flash##ptc"), gr.color_flash);
			zui::color_picker(ecrypt("smoke##ptc"), gr.color_smoke);
			zui::color_picker(ecrypt("molotov##ptc"), gr.color_molotov);
			zui::color_picker(ecrypt("decoy##ptc"), gr.color_decoy);
		}

		zui::end_group_box();
	}
}
