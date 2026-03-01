#pragma once

/**
 * @file KeyCodes.h
 * @brief Platform-agnostic key code constants
 *
 * Provides unified key constants that map to Win32 VK_* codes on Windows
 * and GLFW key codes on other platforms. This allows InputContext and other
 * viewport-owned systems to work without directly depending on GLFW headers.
 *
 * For letter and digit keys, Win32 VK codes and GLFW codes share the same
 * ASCII values. For special keys (Escape, Shift, Control, F-keys, arrows),
 * the values differ between platforms.
 */

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#else
    #include <GLFW/glfw3.h>
#endif

namespace fresh
{

namespace key
{

// --- Letter keys (ASCII on both platforms) ---
#ifdef _WIN32
    inline constexpr int A = 'A';
    inline constexpr int B = 'B';
    inline constexpr int C = 'C';
    inline constexpr int D = 'D';
    inline constexpr int E = 'E';
    inline constexpr int F = 'F';
    inline constexpr int G = 'G';
    inline constexpr int H = 'H';
    inline constexpr int I = 'I';
    inline constexpr int J = 'J';
    inline constexpr int K = 'K';
    inline constexpr int L = 'L';
    inline constexpr int M = 'M';
    inline constexpr int N = 'N';
    inline constexpr int O = 'O';
    inline constexpr int P = 'P';
    inline constexpr int Q = 'Q';
    inline constexpr int R = 'R';
    inline constexpr int S = 'S';
    inline constexpr int T = 'T';
    inline constexpr int U = 'U';
    inline constexpr int V = 'V';
    inline constexpr int W = 'W';
    inline constexpr int X = 'X';
    inline constexpr int Y = 'Y';
    inline constexpr int Z = 'Z';
#else
    inline constexpr int A = GLFW_KEY_A;
    inline constexpr int B = GLFW_KEY_B;
    inline constexpr int C = GLFW_KEY_C;
    inline constexpr int D = GLFW_KEY_D;
    inline constexpr int E = GLFW_KEY_E;
    inline constexpr int F = GLFW_KEY_F;
    inline constexpr int G = GLFW_KEY_G;
    inline constexpr int H = GLFW_KEY_H;
    inline constexpr int I = GLFW_KEY_I;
    inline constexpr int J = GLFW_KEY_J;
    inline constexpr int K = GLFW_KEY_K;
    inline constexpr int L = GLFW_KEY_L;
    inline constexpr int M = GLFW_KEY_M;
    inline constexpr int N = GLFW_KEY_N;
    inline constexpr int O = GLFW_KEY_O;
    inline constexpr int P = GLFW_KEY_P;
    inline constexpr int Q = GLFW_KEY_Q;
    inline constexpr int R = GLFW_KEY_R;
    inline constexpr int S = GLFW_KEY_S;
    inline constexpr int T = GLFW_KEY_T;
    inline constexpr int U = GLFW_KEY_U;
    inline constexpr int V = GLFW_KEY_V;
    inline constexpr int W = GLFW_KEY_W;
    inline constexpr int X = GLFW_KEY_X;
    inline constexpr int Y = GLFW_KEY_Y;
    inline constexpr int Z = GLFW_KEY_Z;
#endif

// --- Special keys ---
#ifdef _WIN32
    inline constexpr int Space        = VK_SPACE;
    inline constexpr int Escape       = VK_ESCAPE;
    inline constexpr int LeftShift    = VK_LSHIFT;
    inline constexpr int RightShift   = VK_RSHIFT;
    inline constexpr int LeftControl  = VK_LCONTROL;
    inline constexpr int RightControl = VK_RCONTROL;
    inline constexpr int LeftAlt      = VK_LMENU;
    inline constexpr int RightAlt     = VK_RMENU;
    inline constexpr int Tab          = VK_TAB;
    inline constexpr int Enter        = VK_RETURN;
    inline constexpr int Backspace    = VK_BACK;
    inline constexpr int Delete       = VK_DELETE;
    inline constexpr int Insert       = VK_INSERT;
    inline constexpr int Home         = VK_HOME;
    inline constexpr int End          = VK_END;
    inline constexpr int PageUp       = VK_PRIOR;
    inline constexpr int PageDown     = VK_NEXT;
#else
    inline constexpr int Space        = GLFW_KEY_SPACE;
    inline constexpr int Escape       = GLFW_KEY_ESCAPE;
    inline constexpr int LeftShift    = GLFW_KEY_LEFT_SHIFT;
    inline constexpr int RightShift   = GLFW_KEY_RIGHT_SHIFT;
    inline constexpr int LeftControl  = GLFW_KEY_LEFT_CONTROL;
    inline constexpr int RightControl = GLFW_KEY_RIGHT_CONTROL;
    inline constexpr int LeftAlt      = GLFW_KEY_LEFT_ALT;
    inline constexpr int RightAlt     = GLFW_KEY_RIGHT_ALT;
    inline constexpr int Tab          = GLFW_KEY_TAB;
    inline constexpr int Enter        = GLFW_KEY_ENTER;
    inline constexpr int Backspace    = GLFW_KEY_BACKSPACE;
    inline constexpr int Delete       = GLFW_KEY_DELETE;
    inline constexpr int Insert       = GLFW_KEY_INSERT;
    inline constexpr int Home         = GLFW_KEY_HOME;
    inline constexpr int End          = GLFW_KEY_END;
    inline constexpr int PageUp       = GLFW_KEY_PAGE_UP;
    inline constexpr int PageDown     = GLFW_KEY_PAGE_DOWN;
#endif

// --- Arrow keys ---
#ifdef _WIN32
    inline constexpr int Up    = VK_UP;
    inline constexpr int Down  = VK_DOWN;
    inline constexpr int Left  = VK_LEFT;
    inline constexpr int Right = VK_RIGHT;
#else
    inline constexpr int Up    = GLFW_KEY_UP;
    inline constexpr int Down  = GLFW_KEY_DOWN;
    inline constexpr int Left  = GLFW_KEY_LEFT;
    inline constexpr int Right = GLFW_KEY_RIGHT;
#endif

// --- Function keys ---
#ifdef _WIN32
    inline constexpr int F1  = VK_F1;
    inline constexpr int F2  = VK_F2;
    inline constexpr int F3  = VK_F3;
    inline constexpr int F4  = VK_F4;
    inline constexpr int F5  = VK_F5;
    inline constexpr int F6  = VK_F6;
    inline constexpr int F7  = VK_F7;
    inline constexpr int F8  = VK_F8;
    inline constexpr int F9  = VK_F9;
    inline constexpr int F10 = VK_F10;
    inline constexpr int F11 = VK_F11;
    inline constexpr int F12 = VK_F12;
#else
    inline constexpr int F1  = GLFW_KEY_F1;
    inline constexpr int F2  = GLFW_KEY_F2;
    inline constexpr int F3  = GLFW_KEY_F3;
    inline constexpr int F4  = GLFW_KEY_F4;
    inline constexpr int F5  = GLFW_KEY_F5;
    inline constexpr int F6  = GLFW_KEY_F6;
    inline constexpr int F7  = GLFW_KEY_F7;
    inline constexpr int F8  = GLFW_KEY_F8;
    inline constexpr int F9  = GLFW_KEY_F9;
    inline constexpr int F10 = GLFW_KEY_F10;
    inline constexpr int F11 = GLFW_KEY_F11;
    inline constexpr int F12 = GLFW_KEY_F12;
#endif

// --- Digit keys (ASCII on both platforms) ---
#ifdef _WIN32
    inline constexpr int Num0 = '0';
    inline constexpr int Num1 = '1';
    inline constexpr int Num2 = '2';
    inline constexpr int Num3 = '3';
    inline constexpr int Num4 = '4';
    inline constexpr int Num5 = '5';
    inline constexpr int Num6 = '6';
    inline constexpr int Num7 = '7';
    inline constexpr int Num8 = '8';
    inline constexpr int Num9 = '9';
#else
    inline constexpr int Num0 = GLFW_KEY_0;
    inline constexpr int Num1 = GLFW_KEY_1;
    inline constexpr int Num2 = GLFW_KEY_2;
    inline constexpr int Num3 = GLFW_KEY_3;
    inline constexpr int Num4 = GLFW_KEY_4;
    inline constexpr int Num5 = GLFW_KEY_5;
    inline constexpr int Num6 = GLFW_KEY_6;
    inline constexpr int Num7 = GLFW_KEY_7;
    inline constexpr int Num8 = GLFW_KEY_8;
    inline constexpr int Num9 = GLFW_KEY_9;
#endif

// --- Mouse buttons ---
#ifdef _WIN32
    inline constexpr int MouseLeft   = VK_LBUTTON;
    inline constexpr int MouseRight  = VK_RBUTTON;
    inline constexpr int MouseMiddle = VK_MBUTTON;
#else
    inline constexpr int MouseLeft   = GLFW_MOUSE_BUTTON_LEFT;
    inline constexpr int MouseRight  = GLFW_MOUSE_BUTTON_RIGHT;
    inline constexpr int MouseMiddle = GLFW_MOUSE_BUTTON_MIDDLE;
#endif

} // namespace key

} // namespace fresh
