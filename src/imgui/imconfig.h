#pragma once

// Disable Unix shell execvp/waitpid functions on PS Vita OS
#define IMGUI_DISABLE_DEFAULT_SHELL_FUNCTIONS 1

// Use stb_sprintf for 100% exception-safe, crash-proof string formatting on PS Vita
#define IMGUI_USE_STB_SPRINTF 1
#define STB_SPRINTF_NOUNALIGNED 1

// Disable debug tools to prevent uninitialized memory access on SceLibc
#define IMGUI_DISABLE_DEBUG_TOOLS 1
