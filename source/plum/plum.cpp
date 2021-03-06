#include "core/log.h"
#include "core/audio.h"
#include "core/screen.h"
#include "core/config.h"
#include "core/engine.h"
#include "core/timer.h"
#include "core/input.h"
#include "script/script.h"

#include <cstdlib>
#include <stdexcept>

int main(int argc, char** argv)
{
    plum::clearLog();
    try
    {
        plum::Config config("plum.cfg");
        auto xres = std::max(config.get<int>("xres", 320), 0);
        auto yres = std::max(config.get<int>("yres", 240), 0);
        auto scale = std::max(config.get<int>("scale", 2), 1);
        auto silent = config.get<bool>("silent", false);
        auto windowed = config.get<bool>("windowed", true);

        plum::Engine engine;
        plum::Keyboard keyboard(engine);
        plum::Mouse mouse(engine);
        plum::Timer timer(engine);
        plum::Audio audio(engine, silent);
        plum::Screen screen(engine, xres, yres, scale, windowed);

        auto hook = engine.addUpdateHook([&]() {
            if(keyboard[plum::KeyTilde].isPressed())
            {
                audio.setPitch(1.0 * plum::Timer::FastForwardMultiplier);
                timer.setSpeed(plum::TimerSpeedFastForward);
            }
            else if(keyboard[plum::KeyLeftShift].isPressed())
            {
                audio.setPitch(1.0 / plum::Timer::SlowMotionDivisor);
                timer.setSpeed(plum::TimerSpeedSlowMotion);
            }
            else
            {
                audio.setPitch(1.0);
                timer.setSpeed(plum::TimerSpeedNormal);
            }
            if((keyboard[plum::KeyLeftAlt].isPressed() || keyboard[plum::KeyRightAlt].isPressed())
            && (keyboard[plum::KeyF4].isPressed() || keyboard[plum::KeyX].isPressed()))
            {
                engine.quit();
            }
        });

        try
        {
            plum::Script script(engine, timer, keyboard, mouse, audio, screen);
            script.run("system.lua");
        }
        catch(const std::runtime_error& e)
        {
            engine.quit(e.what());
        }
    }
    catch(const plum::SystemExit& e)
    {
        if(e.status())
        {
            std::exit(e.status());
        }
    }

    return 0;
}


#ifdef _WIN32
#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}

#endif