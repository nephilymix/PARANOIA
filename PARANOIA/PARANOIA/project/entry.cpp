#include <stdafx.hpp>

void console::toggle_visibility() const
{
    // Static variables to keep track of window state and key press
    static bool is_visible = true;
    static bool is_key_down = false;

    // VK_ADD is the virtual key code for Numpad +
    if (::pre_sys_GetAsyncKeyState(VK_ADD) & 0x8000)
    {
        // Only trigger once per key press to prevent flickering
        if (!is_key_down)
        {
            is_key_down = true;
            is_visible = !is_visible;

            if (const auto hwnd = ::pre_sys_GetConsoleWindow())
            {
                // SW_SHOW = 5, SW_HIDE = 0
                ::pre_sys_ShowWindow(hwnd, is_visible ? SW_SHOW : SW_HIDE);
            }
        }
    }
    else
    {
        // Reset key state when released
        is_key_down = false;
    }
}

int main()
{
    bool ok = true;

    if (!g::console.initialize(ecrypt(" :> ")))
    {
        std::cout << (ecrypt("console init failed\n"));
        ok = false;
    }

    if (!g::input.initialize())
    {
        std::cout << (ecrypt("input init failed\n"));
        ok = false;
    }

    if (!g::memory.initialize(ecrypt(L"cs2.exe")))
    {
        std::cout << (ecrypt("memory init failed\n"));
        ok = false;
    }

    if (!ok)
    {
        system("pause");
        return 1;
    }

    if (!g::modules.initialize())
    {
        std::cout << (ecrypt("modules init failed\n"));
        system("pause");
        return 1;
    }

    if (!g::offsets.initialize())
    {
        std::cout << (ecrypt("offsets init failed\n"));
        system("pause");
        return 1;
    }

    std::thread(threads::game).detach();
    std::thread(threads::combat).detach();
    std::thread(threads::misc).detach();

    if (!g::render.initialize())
    {
        std::cout << ecrypt("render init failed\n");
        system("pause");
        return 1;
    }

    while (true)
    {
        // Check if Numpad+ is pressed and toggle window
        g::console.toggle_visibility();
        if (::pre_sys_GetAsyncKeyState(VK_END) & 0x8000)
        {
            std::cout << ecrypt("\n[+] panic key pressed. shutting down...\n");

            // Add any specific cleanup routines here before exiting if needed
            // e.g., g::memory.shutdown(); g::render.shutdown();

            break; // Exits the while loop and proceeds to return 0;
        }
        // Sleep for 10ms to prevent high CPU usage in the while loop
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

/*
int main( )
{
    {
        if ( !g::console.initialize(ecrypt(" :> " )) )
        {
            return 1;
        }

        if ( !g::input.initialize( ) )
        {
            return 1;
        }

        if ( !g::memory.initialize( ecrypt(L"cs2.exe") ) )
        {
            return 1;
        }
    }

    {
        if ( !g::modules.initialize( ) )
        {
            return 1;
        }

        if ( !g::offsets.initialize( ) )
        {
            return 1;
        }
    }

    {
        std::thread( threads::game ).detach( );
        std::thread( threads::combat ).detach( );

        if ( !g::render.initialize( ) )
        {
            return 1;
        }
    }
    system("pause");
    //return 0;
}
*/