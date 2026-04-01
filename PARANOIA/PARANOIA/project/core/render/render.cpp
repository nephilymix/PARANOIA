#include <stdafx.hpp>
//use model resources only here
#include <resources/models/ctm_sas.h>
#include <resources/models/ctm_fbi.h>
#include <resources/models/ctm_heavy.h>
#include <resources/models/ctm_swat_variante.h>
#include <resources/models/ctm_st6_variante.h>
#include <resources/models/ctm_gendarmerie_varianta.h>
#include <resources/models/ctm_diver_varianta.h>
#include <resources/models/tm_phoenix_heavy.h>
#include <resources/models/tm_phoenix.h>
#include <resources/models/tm_professional_varf.h>
#include <resources/models/tm_leet_varianta.h>
#include <resources/models/tm_jumpsuit_varianta.h>
#include <resources/models/tm_jungle_raider_varianta.h>
#include <resources/models/tm_balkan_variantf.h>

static const wchar_t* const sz_class_name = L"class";

bool render::initialize()
{
    g::console.print("[render] starting initialization...");

    if (!this->register_window_class())
    {
        g::console.print("[render] failed to register window class.");
        return false;
    }

    g::console.print("[render] window class registered successfully.");

    const auto screen_w = ::pre_sys_GetSystemMetrics(SM_CXSCREEN);
    const auto screen_h = ::pre_sys_GetSystemMetrics(SM_CYSCREEN);

    // Replaced k_class_name with sz_class_name and GetModuleHandleW with the wrapper
    this->m_hwnd = ::pre_sys_CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE, sz_class_name, sz_class_name, WS_POPUP, 0, 0, screen_w, screen_h, nullptr, nullptr, ::pre_sys_GetModuleHandleW(nullptr), nullptr);
    if (!this->m_hwnd)
    {
        g::console.print("[render] failed to create window. HWND is null.");
        return false;
    }

    g::console.print("[render] window created successfully.");

    ::pre_sys_SWDA(this->m_hwnd, WDA_EXCLUDEFROMCAPTURE);

    constexpr MARGINS margins{ -1, -1, -1, -1 };
    ::pre_sys_DwmExtendFrameIntoClientArea(this->m_hwnd, &margins);
    ::pre_sys_SetLayeredWindowAttributes(this->m_hwnd, 0, 255, LWA_ALPHA);
    ::pre_sys_ShowWindow(this->m_hwnd, SW_SHOW);
    ::pre_sys_UpdateWindow(this->m_hwnd);

    g::console.print("[render] setting up d3d...");

    if (!this->setup_d3d())
    {
        g::console.print("[render] setup_d3d failed.");
        return false;
    }

    g::console.print("[render] initialized completely.");

    this->run();

    return true;
}

bool render::register_window_class()
{
    WNDCLASSEXW wc{};

    // Replaced k_class_name with sz_class_name
    if (::pre_sys_GetClassInfoExW(::pre_sys_GetModuleHandleW(nullptr), sz_class_name, &wc))
    {
        g::console.print("[render] window class already exists, skipping registration.");
        return true;
    }

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = ::pre_sys_GetModuleHandleW(nullptr);
    wc.hbrBackground = static_cast<HBRUSH>(::pre_sys_GetStockObject(BLACK_BRUSH));
    wc.hCursor = ::pre_sys_LoadCursorA(nullptr, (LPCSTR)IDC_ARROW); // Cast added to avoid type mismatch
    wc.lpszClassName = sz_class_name; // Use valid string

    this->m_atom = ::pre_sys_RegisterClassExW(&wc);

    if (this->m_atom == 0)
    {
        const DWORD err = ::pre_sys_GetLastError();

        char log_buf[128];
        std::snprintf(log_buf, sizeof(log_buf), ecrypt("[render] RegisterClassExW returned 0. GetLastError: %lu"), err);
        std::cout << log_buf;
    }

    return this->m_atom != 0;
}

void render::run()
{
    constexpr float clear[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
    MSG msg{};

    g::console.print("[render] entering main message loop.");

    while (true)
    {
        while (::pre_sys_PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                g::console.print("[render] WM_QUIT received, exiting loop.");
                return;
            }

            ::pre_sys_TranslateMessage(&msg);
            ::pre_sys_DispatchMessageW(&msg);
        }

        this->update_input_window();

        this->m_context->OMSetRenderTargets(1, &this->m_rtv, nullptr);
        this->m_context->ClearRenderTargetView(this->m_rtv, clear);

        zdraw::begin_frame();
        {
            if (systems::g_local.valid())
            {
                systems::g_view.update();
                features::esp::g_player.on_render();
                features::esp::g_item.on_render();
                features::esp::g_projectile.on_render();
                features::misc::g_grenades.on_render();
                features::combat::g_legit.on_render();
                features::misc::g_spectators.render();
                features::esp::g_mesh_renderer.flush();
            }

            g::menu.draw();
        }
        zdraw::end_frame();

        if (FAILED(this->m_swap_chain->Present(0, 0)))
        {
            g::console.print("[render] swap_chain->Present failed, breaking loop.");
            break;
        }
    }
}

void render::update_input_window()
{
    const auto style = ::pre_sys_GetWindowLongW(this->m_hwnd, GWL_EXSTYLE);

    const HWND game_hwnd = ::pre_sys_FindWindowA(ecrypt("SDL_app"), ecrypt("Counter-Strike 2"));
    const HWND foreground = ::pre_sys_GetForegroundWindow();

    // Check if the game or our overlay is currently the active window
    const bool is_active = (game_hwnd != nullptr && (foreground == game_hwnd || foreground == this->m_hwnd));

    static bool is_visible = true;

    if (is_active)
    {
        // Restore overlay visibility when tabbing back into the game
        if (!is_visible)
        {
            ::pre_sys_ShowWindow(this->m_hwnd, SW_SHOW);
            is_visible = true;
        }

        // Toggle click-through state based on menu open state
        if (g::menu.is_open())
        {
            ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
        }
        else
        {
            ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT);
        }
    }
    else
    {
        // Completely hide the overlay window when alt-tabbed or game is minimized
        if (is_visible)
        {
            ::pre_sys_ShowWindow(this->m_hwnd, SW_HIDE);
            is_visible = false;
        }
    }
}

/*
void render::update_input_window()
{
    const auto style = ::pre_sys_GetWindowLongW(this->m_hwnd, GWL_EXSTYLE);

    // Setup active window check using proper wide strings and macro
    const HWND game_hwnd = ::pre_sys_FindWindowA(nullptr, ecrypt("Counter-Strike 2"));
    const HWND foreground = ::pre_sys_GetForegroundWindow();
    const bool is_active = (game_hwnd != nullptr && foreground == game_hwnd);

    // Only make the overlay clickable if the menu is open AND the game is in focus
    if (g::menu.is_open() && is_active)
    {
        ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
    }
    else
    {
        ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT);
    }
}
*/

/*

void render::update_input_window()
{
    const auto style = ::pre_sys_GetWindowLongW(this->m_hwnd, GWL_EXSTYLE);

    if (g::menu.is_open())
    {
        ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style & ~WS_EX_TRANSPARENT);
    }
    else
    {
        ::pre_sys_SetWindowLongW(this->m_hwnd, GWL_EXSTYLE, style | WS_EX_TRANSPARENT);
    }
}
*/

bool render::setup_d3d()
{
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 2;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow = this->m_hwnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL levels[]{ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL selected{};

    // buffer for formatting error codes
    char log_buf[128];

    g::console.print("[render] creating d3d11 device and swap chain...");

    HRESULT hr_device = pre_sys_D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS, levels, 2, D3D11_SDK_VERSION, &desc, &this->m_swap_chain, &this->m_device, &selected, &this->m_context);
    // chams
    features::esp::g_mesh_renderer.initialize(this->m_device, this->m_context);

    // load all models from byte arrays into vram
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_sas", ctm_sas, sizeof(ctm_sas));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_fbi", ctm_fbi, sizeof(ctm_fbi));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_heavy", ctm_heavy, sizeof(ctm_heavy));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_swat_variante", ctm_swat_variante, sizeof(ctm_swat_variante));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_st6_variante", ctm_st6_variante, sizeof(ctm_st6_variante));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_gendarmerie_varianta", ctm_gendarmerie_varianta, sizeof(ctm_gendarmerie_varianta));
    features::esp::g_mesh_renderer.load_mesh_from_memory("ctm_diver_varianta", ctm_diver_varianta, sizeof(ctm_diver_varianta));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_phoenix_heavy", tm_phoenix_heavy, sizeof(tm_phoenix_heavy));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_phoenix", tm_phoenix, sizeof(tm_phoenix));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_professional_varf", tm_professional_varf, sizeof(tm_professional_varf));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_leet_varianta", tm_leet_varianta, sizeof(tm_leet_varianta));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_jumpsuit_varianta", tm_jumpsuit_varianta, sizeof(tm_jumpsuit_varianta));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_jungle_raider_varianta", tm_jungle_raider_varianta, sizeof(tm_jungle_raider_varianta));
    features::esp::g_mesh_renderer.load_mesh_from_memory("tm_balkan_variantf", tm_balkan_variantf, sizeof(tm_balkan_variantf));

    if (FAILED(hr_device))
    {
        std::snprintf(log_buf, sizeof(log_buf), ecrypt("[render] pre_sys_D3D11CreateDeviceAndSwapChain failed. HRESULT: 0x%08lX"), hr_device);
        std::cout << log_buf;
        //g::console.print(log_buf);
        return false;
    }

    g::console.print("[render] getting swap chain buffer...");

    ID3D11Texture2D* back_buffer{};
    HRESULT hr_buffer = this->m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

    if (FAILED(hr_buffer))
    {
        std::snprintf(log_buf, sizeof(log_buf), ecrypt("[render] m_swap_chain->GetBuffer failed. HRESULT: 0x%08lX"), hr_buffer);
        std::cout << log_buf;
        //g::console.print(log_buf);
        return false;
    }

    g::console.print("[render] creating render target view...");

    HRESULT hr_rtv = this->m_device->CreateRenderTargetView(back_buffer, nullptr, &this->m_rtv);

    D3D11_TEXTURE2D_DESC bb_desc{};
    back_buffer->GetDesc(&bb_desc);
    back_buffer->Release();

    if (FAILED(hr_rtv))
    {
        std::snprintf(log_buf, sizeof(log_buf), ecrypt("[render] CreateRenderTargetView failed. HRESULT: 0x%08lX"), hr_rtv);
        std::cout << log_buf;
        //g::console.print(log_buf);
        return false;
    }

    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(bb_desc.Width);
    vp.Height = static_cast<float>(bb_desc.Height);
    vp.MaxDepth = 1.0f;
    this->m_context->RSSetViewports(1, &vp);

    g::console.print("[render] initializing zdraw...");

    if (!zdraw::initialize(this->m_device, this->m_context))
    {
        g::console.print("[render] zdraw::initialize failed.");
        return false;
    }

    g::console.print("[render] initializing zui...");
    zui::initialize(this->m_hwnd);

    g::console.print("[render] loading fonts...");
    {
        this->m_fonts.lexend_10 = zdraw::add_font_from_memory(std::span(reinterpret_cast<const std::byte*>(lexend), sizeof(lexend)), 18.0f, 512, 512);
        if (!this->m_fonts.lexend_10)
        {
            g::console.print("[render] warning: failed to load lexend_10 font.");
        }
        else
        {
            g::console.print("[render] lexend_10 font loaded successfully.");
        }

        this->m_fonts.weapons_15 = zdraw::add_font_from_memory(std::span(reinterpret_cast<const std::byte*>(resources::fonts::weapons), sizeof(resources::fonts::weapons)), 16.0f, 512, 512);
        if (!this->m_fonts.weapons_15)
        {
            g::console.print("[render] warning: failed to load weapons_15 font.");
        }
        else
        {
            g::console.print("[render] weapons_15 font loaded successfully.");
        }
    }

    g::console.print("[render] d3d setup complete.");
    return true;
}

LRESULT CALLBACK render::wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    zui::process_wndproc_message(msg, wp, lp);

    if (msg == WM_DESTROY)
    {
        ::pre_sys_PostQuitMessage(0);
        return 0;
    }

    return ::pre_sys_DefWindowProcW(hwnd, msg, wp, lp);
}