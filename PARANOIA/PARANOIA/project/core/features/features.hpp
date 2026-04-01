#pragma once

namespace features {

	namespace combat {

		class legit
		{
		public:
			void on_render( );
			void tick( );

		private:
			struct target
			{
				const systems::collector::player* player{};
				systems::bones::data bones{};
				math::vector3 aim_point{};
				int hitbox{ -1 };
				int hitgroup{ -1 };
				float damage{};
				float fov{};
				bool penetrated{};
			};

			[[nodiscard]] target select_target( const math::vector3& eye_pos, const math::vector3& view_angles, const std::vector<systems::collector::player>& players, const settings::combat::group_config& cfg ) const;
			[[nodiscard]] math::vector3 get_aim_point( const math::vector3& eye_pos, const systems::collector::player& player, const systems::bones::data& bones, const settings::combat::group_config& cfg, float& out_damage, int& out_hitbox, bool& out_penetrated ) const;

			[[nodiscard]] float get_fov( const math::vector3& view_angles, const math::vector3& eye_pos, const math::vector3& target_pos ) const;
			[[nodiscard]] float get_fov_radius( const math::vector3& eye_pos, const math::vector3& view_angles, float fov_degrees ) const;

			void draw_fov( const math::vector3& eye_pos, const math::vector3& view_angles, const settings::combat::aimbot& cfg );
			void aimbot( const math::vector3& eye_pos, const math::vector3& view_angles, const target& tgt, const settings::combat::aimbot& cfg );

			struct trigger_result
			{
				const systems::collector::player* player{};
				systems::bones::data bones{};
				int hitbox{ -1 };
				int hitgroup{ -1 };
				float damage{};
				bool penetrated{};
			};

			[[nodiscard]] trigger_result trace_crosshair( const math::vector3& eye_pos, const math::vector3& view_angles, const std::vector<systems::collector::player>& players, const settings::combat::triggerbot& cfg ) const;
			void triggerbot( const math::vector3& eye_pos, const math::vector3& view_angles, const std::vector<systems::collector::player>& players, const settings::combat::triggerbot& cfg );

			void apply_autostop( );
			void release_autostop( );

			animation::spring m_fov_alpha{};
			random::valve_rng m_rng{};
			bool m_rng_seeded{ false };

			math::vector2 m_aim_error{};

			float m_trigger_delay_end{ 0.0f };
			bool m_trigger_waiting{ false };
			bool m_trigger_held{ false };
			float m_trigger_release_time{ 0.0f };

			bool m_autostop_active{ false };
			std::chrono::steady_clock::time_point m_autostop_start{};
			std::vector<std::uint16_t> m_autostop_keys{};
		};

		class shared
		{
		public:
			struct context
			{
				std::uintptr_t weapon;
				std::uintptr_t weapon_vdata;
				std::uint32_t weapon_type;
				std::uint16_t item_def_idx;
				int num_bullets;
				float accuracy_penalty;
				float inaccuracy;
				float spread;
				float recoil_index;
				bool is_reloading;
				bool is_full_auto;
				bool is_scoped;
				bool weapon_ready;
				float current_time;
				float cycle_time;
				float last_shot_time;
				bool valid;
			};

			class penetration
			{
			public:
				struct weapon_data
				{
					float damage;
					float penetration;
					float range_modifier;
					float range;
					float armor_ratio;
					float headshot_multiplier;
				};

				struct result
				{
					float damage;
					int hitbox;
					bool penetrated;
				};

				void prepare( std::uintptr_t weapon_vdata, std::uintptr_t weapon );

				[[nodiscard]] bool run( const math::vector3& start, const math::vector3& end, const systems::collector::player& target, const systems::bones::data& bones, result& out ) const;
				[[nodiscard]] float get_max_damage( int hitgroup, int target_armor, bool has_helmet, int target_team ) const;
				[[nodiscard]] const weapon_data& get_weapon_data( ) const { return this->m_weapon_data; }

			private:
				weapon_data m_weapon_data{};
			};

			void tick( );

			[[nodiscard]] const context& ctx( ) const { return this->m_ctx; }
			[[nodiscard]] const penetration& pen( ) const { return this->m_pen; }

			[[nodiscard]] float calculate_hitchance( const math::vector3& eye_pos, const math::vector3& aim_angle, const systems::collector::player& target, const systems::bones::data& bones ) const;
			[[nodiscard]] std::uint32_t get_spread_seed( const math::vector3& angles, int tick ) const;
			[[nodiscard]] math::vector2 calculate_spread( int seed, float accuracy, float spread, float recoil_index, int item_def_idx, int num_bullets ) const;
			[[nodiscard]] math::vector3 extrapolate_stop( const math::vector3& pos ) const;
			[[nodiscard]] bool is_sniper_accurate( ) const;
			[[nodiscard]] float get_prediction_time( ) const;

		private:
			[[nodiscard]] float get_spread( std::uintptr_t weapon_vdata ) const;
			[[nodiscard]] float get_inaccuracy( std::uintptr_t pawn, std::uintptr_t weapon, std::uintptr_t weapon_vdata, const math::vector3& eye_angles, float accuracy_penalty ) const;

			context m_ctx{};
			penetration m_pen{};
			mutable std::shared_mutex m_ctx_mutex{};
		};

		inline legit g_legit{};
		inline shared g_shared{};

	} // namespace combat

	namespace esp {

		class player
		{
		public:
			void on_render( );

		private:
			struct draw_offsets
			{
				float left{ 0.0f };
				float top{ 0.0f };
				float bottom{ 0.0f };
				float right{ 0.0f };
			};

			void add_box( const systems::bounds::data& bounds, const settings::esp::player::box& cfg, bool is_visible );
			void add_skeleton( const systems::bones::data& bones, const settings::esp::player::skeleton& cfg, bool is_visible );
			void add_hitboxes( const systems::bones::data& bones, const systems::collector::player& player, const settings::esp::player::hitboxes& cfg, float current_time );
			void add_health_bar( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::health_bar& cfg, draw_offsets& offsets );
			void add_ammo_bar( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::ammo_bar& cfg, draw_offsets& offsets );
			void add_name( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::name& cfg, draw_offsets& offsets );
			void add_weapon( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::weapon& cfg, draw_offsets& offsets );
			void add_flags( const systems::bounds::data& bounds, const systems::collector::player& player, const settings::esp::player::info_flags& cfg, draw_offsets& offsets );

			[[nodiscard]] static std::string get_weapon_icon( const std::string& weapon_name );

			struct animation_data
			{
				animation::spring health{};
				animation::spring ammo{};
				bool initialized{ false };
				float last_damage_time{ 0.0f };
				int last_health{ 100 };
			};

			std::unordered_map<std::uintptr_t, animation_data> m_animations{};
		};

		class item
		{
		public:
			void on_render( );

		private:
			enum class category : std::uint8_t { rifle, smg, shotgun, sniper, pistol, heavy, grenade, utility };

			void add_icon( const math::vector2& screen, const systems::collector::item& item, const settings::esp::item::icon& cfg, float& y_offset );
			void add_name( const math::vector2& screen, const systems::collector::item& item, const settings::esp::item::name& cfg, float& y_offset );
			void add_ammo( const math::vector2& screen, const systems::collector::item& item, const settings::esp::item::ammo& cfg, float& y_offset );

			[[nodiscard]] bool passes_filter( systems::collector::item_subtype subtype, const settings::esp::item::filters& filters ) const;
			[[nodiscard]] category get_category( systems::collector::item_subtype subtype ) const;
			[[nodiscard]] std::string get_icon( systems::collector::item_subtype subtype ) const;
			[[nodiscard]] std::string get_display_name( systems::collector::item_subtype subtype ) const;
		};

		class projectile
		{
		public:
			void on_render( );

		private:
			void draw_timer( const math::vector2& screen, float& y_offset, float remaining, float frac, const settings::esp::projectile& cfg ) const;
			void draw_inferno_bounds( const systems::collector::projectile& proj, const settings::esp::projectile& cfg ) const;
			[[nodiscard]] zdraw::rgba get_color( systems::collector::projectile_subtype type, const settings::esp::projectile& cfg ) const;
			[[nodiscard]] std::string get_icon( systems::collector::projectile_subtype type ) const;
			[[nodiscard]] std::string get_name( systems::collector::projectile_subtype type ) const;
			[[nodiscard]] zdraw::rgba lerp_color( const zdraw::rgba& a, const zdraw::rgba& b, float t ) const;
		};

		inline player g_player{};
		inline item g_item{};
		inline projectile g_projectile{};

	} // namespace esp

	namespace misc {

		class grenades
		{
		public:
			void on_render( );

		private:
			struct trajectory
			{
				std::vector<math::vector3> points{};
				std::vector<math::vector3> bounces{};
				math::vector3 end_pos{};
				float duration{};
				int end_tick{ -1 };
				bool valid{ false };
			};

			struct in_flight_grenade
			{
				std::uintptr_t entity{};
				std::uintptr_t weapon_hash{};
				trajectory traj{};
				std::chrono::steady_clock::time_point throw_time{};
				std::chrono::steady_clock::time_point detonate_time{};
				std::chrono::steady_clock::time_point last_seen{};
				bool detonated{ false };
				bool effect_started{ false };
				bool corrected{ false };
			};

			[[nodiscard]] bool can_predict( ) const;
			void update_weapon_properties( );
			void setup_throw( math::vector3& origin, math::vector3& velocity );

			void update_in_flight( );
			[[nodiscard]] std::uintptr_t hash_from_projectile( systems::collector::projectile_subtype type ) const;
			[[nodiscard]] zdraw::rgba color_for_type( std::uintptr_t weapon_hash ) const;

			void simulate( const math::vector3& start, const math::vector3& velocity, trajectory& out );
			void step_simulation( math::vector3& pos, math::vector3& vel, systems::bvh::trace_result& trace );
			void resolve_collision( const systems::bvh::trace_result& trace, math::vector3& pos, math::vector3& vel );
			[[nodiscard]] bool should_detonate( const math::vector3& vel, int tick ) const;
			void render_trajectory( const trajectory& traj, float alpha, std::uintptr_t weapon_hash ) const;

			std::uintptr_t m_weapon_vdata{};
			std::uintptr_t m_weapon_hash{};
			float m_throw_velocity{};
			float m_detonate_time{ 1.5f };
			float m_velocity_threshold{ 0.1f };

			std::vector<in_flight_grenade> m_in_flight{};
			std::chrono::steady_clock::time_point m_last_throw_time{};
			bool m_was_holding{ false };

			float m_sv_gravity{};
			float m_molotov_max_slope_z{};

			static constexpr auto tick_interval{ 1.0f / 64.0f };
			static constexpr auto gravity_scale{ 0.4f };
			static constexpr auto elasticity{ 0.45f };
			static constexpr auto max_ticks{ 1024 };
			static constexpr auto ticks_per_point{ 4 };
			static constexpr auto throw_cooldown{ 1.0f };
			static constexpr auto missing_grace{ 0.1f };
		};

		class spectators {
		public:
			void update();
			void render();
		private:
			std::vector<std::string> m_spectators{};
			std::mutex m_mutex{};
		};

		inline spectators g_spectators{};

		class impacts
		{
		public:

		};

		inline grenades g_grenades{};
		inline impacts g_impacts{};

	} // namespace misc

} // namespace features