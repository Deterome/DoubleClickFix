#include <Windows.h>

HHOOK g_mouseHook = nullptr;

bool g_processingLeftClick = true;
bool g_processingRightClick = false;

DWORD g_lastLeftMouseDownTime = 0;
DWORD g_lastRightMouseDownTime = 0;

const DWORD DEBOUNCE_TIME_MS = 100;


inline bool ShouldBlockMouseAction(DWORD time, DWORD lastMouseDownTime) {
    return time - lastMouseDownTime < DEBOUNCE_TIME_MS;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam != WM_LBUTTONDOWN && wParam != WM_RBUTTONDOWN) {
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }

        MSLLHOOKSTRUCT* pMouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

        if (pMouseInfo->flags & LLMHF_INJECTED) {
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        } 

        if (wParam == WM_LBUTTONDOWN && g_processingLeftClick) {
            if (ShouldBlockMouseAction(pMouseInfo->time, g_lastLeftMouseDownTime)) {
                return 1;
            } else {
                g_lastLeftMouseDownTime = pMouseInfo->time;
            }
        } 
        
        if (wParam == WM_RBUTTONDOWN && g_processingRightClick) {
            if (ShouldBlockMouseAction(pMouseInfo->time, g_lastRightMouseDownTime)) {
                return 1;
            } else {
                g_lastRightMouseDownTime = pMouseInfo->time;
            }
        } 
    }
    
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_mouseHook = SetWindowsHookEx(
        WH_MOUSE_LL,
        LowLevelMouseProc,
        nullptr,
        0
    );

    if (!g_mouseHook) {
        MessageBox(
            nullptr,
            "Unable to install mouse hook",
            "Error",
            MB_ICONERROR
        );
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(g_mouseHook);
    return 0;
}