Run `python3 check.py` in a disposable checkout with Pico SDK, CMake, Make and
arm-none-eabi installed (PICO_SDK_PATH and PATH set). It configures the actual
platform project, checks the default Pico/memory values, switches Pico -> Pico 2
-> Pico W -> Pico 2 W -> Pico in the same build directory, and checks memory
and flash geometry overrides. This changes
the checkout's build/ cache; it does not flash hardware or build firmware.

The original e35ad7c default has a trailing space, producing an empty TOTALMEM;
with an explicit board it instead fails when changing CPU platforms without
clearing the cache. Direct CMake users should use separate board build folders.
