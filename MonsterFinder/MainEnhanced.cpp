#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")

#define _CRT_SECURE_NO_WARNINGS
#include "MonsterFinderEnhanced.h"
#include <commctrl.h>

// ==========================================
//           GLOBAL VARIABLES
// ==========================================

const char* WINDOW_NAME = "RF Online";
MonsterFinderBot* g_bot = nullptr;
HWND g_hMainWnd = NULL;
HWND g_hStatusLabel = NULL;
HWND g_hStatsLabel = NULL;
HWND g_hLogText = NULL;

// GUI Elements
HWND g_btnStart, g_btnStop, g_btnPause, g_btnExit;
HWND g_btnSetMonster, g_btnReloadConfig, g_btnClearLog;
HWND g_chkAutoLoot, g_chkShowBoxes, g_chkAutoSkill, g_chkRandomDelay;
HWND g_cboTargetMode;

// Fonts
HFONT g_hTitleFont = NULL;
HFONT g_hBoldFont = NULL;
HFONT g_hNormalFont = NULL;
HFONT g_hButtonFont = NULL;
HBRUSH g_hStatusBrush = NULL;

// ==========================================
//           GUI HELPERS
// ==========================================

void UpdateStatus(const char* text) {
    if (g_hStatusLabel) {
        SetWindowTextA(g_hStatusLabel, text);
    }
}

void UpdateStats(const string& statsText) {
    if (g_hStatsLabel) {
        SetWindowTextA(g_hStatsLabel, statsText.c_str());
    }
}

void AddLog(const string& message) {
    if (g_hLogText) {
        int len = GetWindowTextLengthA(g_hLogText);
        SendMessageA(g_hLogText, EM_SETSEL, len, len);
        SendMessageA(g_hLogText, EM_REPLACESEL, FALSE, (LPARAM)message.c_str());
        SendMessageA(g_hLogText, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
        
        // Auto-scroll
        SendMessageA(g_hLogText, EM_SETSEL, len + message.length() + 2, len + message.length() + 2);
        SendMessageA(g_hLogText, EM_SCROLLCARET, 0, 0);
    }
}

void RefreshStatsDisplay() {
    if (g_bot && g_bot->IsRunning()) {
        string stats = g_bot->GetStatsString();
        UpdateStats(stats);
    }
}

// ==========================================
//           WINDOW PROCEDURE
// ==========================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static bool g_statsUpdateEnabled = false;
    
    switch (msg) {
    case WM_CREATE:
    {
        // Initialize common controls
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);
        
        // Create fonts
        g_hTitleFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        
        g_hBoldFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        
        g_hNormalFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        
        g_hButtonFont = CreateFont(12, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        
        // Create status brush (light blue)
        g_hStatusBrush = CreateSolidBrush(RGB(230, 240, 255));
        
        // Title with better styling
        HWND hTitle = CreateWindowA("STATIC", "Farming TAI - DedeProjectDev", 
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            10, 10, 700, 35, hwnd, NULL, NULL, NULL);
        SendMessageA(hTitle, WM_SETFONT, (WPARAM)g_hTitleFont, TRUE);
        
        // Status Label with background
        g_hStatusLabel = CreateWindowA("STATIC", "Status: Ready", 
            WS_VISIBLE | WS_CHILD | SS_LEFT | SS_CENTERIMAGE,
            10, 50, 700, 30, hwnd, NULL, NULL, NULL);
        SendMessageA(g_hStatusLabel, WM_SETFONT, (WPARAM)g_hBoldFont, TRUE);
        
        // Group Box: Configuration
        CreateWindowA("BUTTON", "Configuration", 
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            10, 90, 700, 50, hwnd, NULL, NULL, NULL);
        
        // Control Buttons Row 1
        g_btnSetMonster = CreateWindowA("BUTTON", "Set Monster Color", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 110, 160, 30, hwnd, (HMENU)101, NULL, NULL);
        SendMessageA(g_btnSetMonster, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        g_btnReloadConfig = CreateWindowA("BUTTON", "Reload Config", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            190, 110, 160, 30, hwnd, (HMENU)102, NULL, NULL);
        SendMessageA(g_btnReloadConfig, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        g_btnClearLog = CreateWindowA("BUTTON", "Clear Log", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            360, 110, 160, 30, hwnd, (HMENU)103, NULL, NULL);
        SendMessageA(g_btnClearLog, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        // Group Box: Bot Control
        CreateWindowA("BUTTON", "Bot Control", 
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            10, 150, 700, 60, hwnd, NULL, NULL, NULL);
        
        // Control Buttons Row 2
        g_btnStart = CreateWindowA("BUTTON", "▶ START (F5)", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 170, 150, 35, hwnd, (HMENU)104, NULL, NULL);
        SendMessageA(g_btnStart, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        g_btnPause = CreateWindowA("BUTTON", "⏸ PAUSE (F7)", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            180, 170, 150, 35, hwnd, (HMENU)105, NULL, NULL);
        SendMessageA(g_btnPause, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        g_btnStop = CreateWindowA("BUTTON", "⏹ STOP (F6)", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            340, 170, 150, 35, hwnd, (HMENU)106, NULL, NULL);
        SendMessageA(g_btnStop, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        g_btnExit = CreateWindowA("BUTTON", "✖ EXIT", 
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            500, 170, 200, 35, hwnd, (HMENU)107, NULL, NULL);
        SendMessageA(g_btnExit, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
        
        // Group Box: Options
        CreateWindowA("BUTTON", "Options", 
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            10, 220, 700, 80, hwnd, NULL, NULL, NULL);
        
        // Options Checkboxes
        HWND hOptionsLabel = CreateWindowA("STATIC", "Features:", 
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 240, 100, 20, hwnd, NULL, NULL, NULL);
        SendMessageA(hOptionsLabel, WM_SETFONT, (WPARAM)g_hBoldFont, TRUE);
        
        g_chkAutoLoot = CreateWindowA("BUTTON", "✓ Auto Loot (F8)", 
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
            20, 260, 160, 25, hwnd, (HMENU)108, NULL, NULL);
        SendMessageA(g_chkAutoLoot, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        SendMessageA(g_chkAutoLoot, BM_SETCHECK, BST_CHECKED, 0);
        
        g_chkShowBoxes = CreateWindowA("BUTTON", "✓ Show Boxes (F9)", 
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
            20, 285, 160, 25, hwnd, (HMENU)109, NULL, NULL);
        SendMessageA(g_chkShowBoxes, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        SendMessageA(g_chkShowBoxes, BM_SETCHECK, BST_CHECKED, 0);
        
        g_chkAutoSkill = CreateWindowA("BUTTON", "✓ Auto Skill", 
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
            200, 260, 160, 25, hwnd, (HMENU)110, NULL, NULL);
        SendMessageA(g_chkAutoSkill, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        SendMessageA(g_chkAutoSkill, BM_SETCHECK, BST_CHECKED, 0);
        
        g_chkRandomDelay = CreateWindowA("BUTTON", "✓ Random Delay", 
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
            200, 285, 160, 25, hwnd, (HMENU)111, NULL, NULL);
        SendMessageA(g_chkRandomDelay, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        SendMessageA(g_chkRandomDelay, BM_SETCHECK, BST_CHECKED, 0);
        
        // Target Mode
        HWND hTargetLabel = CreateWindowA("STATIC", "Target Mode:", 
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            380, 240, 120, 20, hwnd, NULL, NULL, NULL);
        SendMessageA(hTargetLabel, WM_SETFONT, (WPARAM)g_hBoldFont, TRUE);
        
        g_cboTargetMode = CreateWindowA("COMBOBOX", "", 
            WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
            380, 265, 200, 200, hwnd, (HMENU)112, NULL, NULL);
        SendMessageA(g_cboTargetMode, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        SendMessageA(g_cboTargetMode, CB_ADDSTRING, 0, (LPARAM)"Priority");
        SendMessageA(g_cboTargetMode, CB_ADDSTRING, 0, (LPARAM)"Nearest");
        SendMessageA(g_cboTargetMode, CB_ADDSTRING, 0, (LPARAM)"Strongest");
        SendMessageA(g_cboTargetMode, CB_ADDSTRING, 0, (LPARAM)"Weakest");
        SendMessageA(g_cboTargetMode, CB_SETCURSEL, 0, 0);
        
        // Hotkey Info
        HWND hHotkeyLabel = CreateWindowA("STATIC", "Hotkeys: F5=Start | F6=Stop | F7=Pause | F8=Toggle Loot | F9=Toggle Boxes | F10=Escape | F11=Reload Config", 
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            10, 310, 700, 20, hwnd, NULL, NULL, NULL);
        SendMessageA(hHotkeyLabel, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        
        // Group Box: Statistics
        CreateWindowA("BUTTON", "Statistics", 
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            10, 340, 700, 100, hwnd, NULL, NULL, NULL);
        
        // Statistics Label
        HWND hStatsTitle = CreateWindowA("STATIC", "Statistics:", 
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 360, 100, 20, hwnd, NULL, NULL, NULL);
        SendMessageA(hStatsTitle, WM_SETFONT, (WPARAM)g_hBoldFont, TRUE);
        
        g_hStatsLabel = CreateWindowA("STATIC", "Waiting to start...", 
            WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 380, 680, 50, hwnd, NULL, NULL, NULL);
        SendMessageA(g_hStatsLabel, WM_SETFONT, (WPARAM)g_hNormalFont, TRUE);
        
        // Group Box: Activity Log
        CreateWindowA("BUTTON", "Activity Log", 
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            10, 450, 700, 180, hwnd, NULL, NULL, NULL);
        
        // Log Text Box
        g_hLogText = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", 
            WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            20, 470, 680, 150, hwnd, NULL, NULL, NULL);
        
        // Set font for log (monospace)
        HFONT hLogFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
            CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
        SendMessageA(g_hLogText, WM_SETFONT, (WPARAM)hLogFont, TRUE);
        
        // Start stats update timer
        SetTimer(hwnd, 1, 1000, NULL);
        g_statsUpdateEnabled = true;
    }
    break;

    case WM_TIMER:
        if (wParam == 1 && g_statsUpdateEnabled) {
            RefreshStatsDisplay();
        }
        break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case 101: // Set Monster Color
        {
            if (g_bot && g_bot->IsRunning()) {
                MessageBoxA(hwnd, "Stop bot first!", "Warning", MB_OK | MB_ICONWARNING);
            } else {
                // Start calibration mode
                UpdateStatus("Click on monster to capture color...");
                AddLog("Starting color calibration...");
                
                bool calibrating = true;
                MSG msg;
                HDC hdc = GetDC(NULL);
                
                AddLog("Click LEFT mouse button on target monster color...");
                
                while (calibrating) {
                    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                        POINT p;
                        GetCursorPos(&p);
                        COLORREF color = GetPixel(hdc, p.x, p.y);
                        
                        int r = GetRValue(color);
                        int g = GetGValue(color);
                        int b = GetBValue(color);
                        
                        char buf[256];
                        sprintf_s(buf, sizeof(buf), "Color captured! R:%d G:%d B:%d", r, g, b);
                        UpdateStatus(buf);
                        
                        AddLog(buf);
                        AddLog("Add this color to monsters.ini file for persistent configuration.");
                        
                        Beep(1000, 200);
                        calibrating = false;
                    }
                    
                    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                        if (msg.message == WM_QUIT || msg.message == WM_CLOSE) {
                            calibrating = false;
                        }
                    }
                    Sleep(20);
                }
                
                ReleaseDC(NULL, hdc);
            }
        }
        break;
        
        case 102: // Reload Config
        {
            if (g_bot) {
                g_bot->LoadAllConfigurations();
                UpdateStatus("Configuration reloaded!");
                AddLog("Configuration reloaded successfully.");
                AddLog("Loaded monsters.ini and monsterfinder_config.ini");
            }
        }
        break;
        
        case 103: // Clear Log
        {
            if (g_hLogText) {
                SetWindowTextA(g_hLogText, "");
                AddLog("Log cleared.");
            }
        }
        break;
        
        case 104: // Start Bot
        {
            if (g_bot && !g_bot->IsRunning()) {
                g_bot->Start();
                UpdateStatus("Bot started! Hunting monsters...");
                AddLog("=== Bot Started ===");
                RefreshStatsDisplay();
            } else if (!g_bot) {
                MessageBoxA(hwnd, "Bot not initialized!", "Error", MB_OK | MB_ICONERROR);
            } else {
                MessageBoxA(hwnd, "Bot already running!", "Info", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
        
        case 105: // Pause Bot
        {
            if (g_bot && g_bot->IsRunning()) {
                if (g_bot->IsPaused()) {
                    g_bot->Resume();
                    UpdateStatus("Bot resumed!");
                    AddLog("Bot resumed.");
                    SetWindowTextA(g_btnPause, "PAUSE (F7)");
                } else {
                    g_bot->Pause();
                    UpdateStatus("Bot paused!");
                    AddLog("Bot paused.");
                    SetWindowTextA(g_btnPause, "RESUME (F7)");
                }
            }
        }
        break;
        
        case 106: // Stop Bot
        {
            if (g_bot && g_bot->IsRunning()) {
                g_bot->Stop();
                UpdateStatus("Bot stopped.");
                AddLog("=== Bot Stopped ===");
                SetWindowTextA(g_btnPause, "⏸ PAUSE (F7)");
                RefreshStatsDisplay();
            }
        }
        break;
        
        case 107: // Exit
        {
            PostQuitMessage(0);
        }
        break;
        
        case 108: // Toggle Auto Loot
        {
            if (g_bot) {
                g_bot->GetConfig().autoLoot = (SendMessageA(g_chkAutoLoot, BM_GETCHECK, 0, 0) == BST_CHECKED);
                string status = g_bot->GetConfig().autoLoot ? "ON" : "OFF";
                UpdateStatus(("Auto Loot: " + status).c_str());
                AddLog("Auto Loot set to: " + status);
            }
        }
        break;
        
        case 109: // Toggle Show Boxes
        {
            if (g_bot) {
                g_bot->GetConfig().showBoxes = (SendMessageA(g_chkShowBoxes, BM_GETCHECK, 0, 0) == BST_CHECKED);
                string status = g_bot->GetConfig().showBoxes ? "ON" : "OFF";
                UpdateStatus(("Show Boxes: " + status).c_str());
                AddLog("Show Boxes set to: " + status);
            }
        }
        break;
        
        case 110: // Toggle Auto Skill
        {
            if (g_bot) {
                g_bot->GetConfig().autoSkill = (SendMessageA(g_chkAutoSkill, BM_GETCHECK, 0, 0) == BST_CHECKED);
                string status = g_bot->GetConfig().autoSkill ? "ON" : "OFF";
                UpdateStatus(("Auto Skill: " + status).c_str());
                AddLog("Auto Skill set to: " + status);
            }
        }
        break;
        
        case 111: // Toggle Random Delay
        {
            if (g_bot) {
                g_bot->GetConfig().randomDelay = (SendMessageA(g_chkRandomDelay, BM_GETCHECK, 0, 0) == BST_CHECKED);
                string status = g_bot->GetConfig().randomDelay ? "ON" : "OFF";
                UpdateStatus(("Random Delay: " + status).c_str());
                AddLog("Random Delay set to: " + status);
            }
        }
        break;
        
        case 112: // Target Mode Changed
        {
            if (g_bot) {
                int sel = SendMessageA(g_cboTargetMode, CB_GETCURSEL, 0, 0);
                char buf[32];
                SendMessageA(g_cboTargetMode, CB_GETLBTEXT, sel, (LPARAM)buf);
                g_bot->GetConfig().targetMode = buf;
                
                char msg[256];
                sprintf_s(msg, sizeof(msg), "Target mode: %s", buf);
                UpdateStatus(msg);
                AddLog(msg);
            }
        }
        break;
        }
    }
    break;

    case WM_DESTROY:
        g_statsUpdateEnabled = false;
        KillTimer(hwnd, 1);
        
        if (g_bot) {
            if (g_bot->IsRunning()) {
                g_bot->Stop();
            }
            delete g_bot;
            g_bot = nullptr;
        }
        
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ==========================================
//           HOTKEY THREAD
// ==========================================

DWORD WINAPI HotkeyThread(LPVOID lpParam) {
    HWND hwnd = (HWND)lpParam;
    
    while (g_bot) {
        Sleep(100);
        
        if (!g_bot) break;
        
        // Check hotkeys
        if (GetAsyncKeyState(VK_F5) & 0x8000) {
            SendMessageA(hwnd, WM_COMMAND, 104, 0); // Start
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F6) & 0x8000) {
            SendMessageA(hwnd, WM_COMMAND, 106, 0); // Stop
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F7) & 0x8000) {
            SendMessageA(hwnd, WM_COMMAND, 105, 0); // Pause/Resume
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F8) & 0x8000) {
            LRESULT state = SendMessageA(g_chkAutoLoot, BM_GETCHECK, 0, 0);
            SendMessageA(g_chkAutoLoot, BM_SETCHECK, (state == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED, 0);
            SendMessageA(hwnd, WM_COMMAND, 108, 0);
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F9) & 0x8000) {
            LRESULT state = SendMessageA(g_chkShowBoxes, BM_GETCHECK, 0, 0);
            SendMessageA(g_chkShowBoxes, BM_SETCHECK, (state == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED, 0);
            SendMessageA(hwnd, WM_COMMAND, 109, 0);
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F10) & 0x8000) {
            if (g_bot && g_bot->IsRunning()) {
                INPUT input = {};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = VK_ESCAPE;
                SendInput(1, &input, sizeof(INPUT));
                input.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
                AddLog("Emergency escape triggered (F10)!");
            }
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_F11) & 0x8000) {
            SendMessageA(hwnd, WM_COMMAND, 102, 0); // Reload Config
            Sleep(500);
        }
    }
    
    return 0;
}

// ==========================================
//           MAIN FUNCTION
// ==========================================

int main() {
    const char* CLASS_NAME = "RF_BOT_ENHANCED";
    
    // Seed random number generator
    srand((unsigned int)time(NULL));
    
    // Find RF Online window
    HWND rfWindow = FindWindowA(NULL, WINDOW_NAME);
    if (!rfWindow) {
        MessageBoxA(NULL, 
            "RF Online window not found!\n"
            "Please start the game first.", 
            "Error", 
            MB_ICONERROR | MB_OK);
        return 0;
    }
    
    // Initialize bot
    g_bot = new MonsterFinderBot();
    if (!g_bot->Initialize(rfWindow)) {
        MessageBoxA(NULL, 
            "Failed to initialize bot!\n"
            "Check console for error details.", 
            "Error", 
            MB_ICONERROR | MB_OK);
        delete g_bot;
        return 0;
    }
    
    // Load configurations
    g_bot->LoadAllConfigurations();
    
    // Create window class
    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        delete g_bot;
        return 0;
    }
    
    // Create main window (slightly larger for better UI)
    g_hMainWnd = CreateWindowExA(
        0, CLASS_NAME, "Farming TAI - DedeProjectDev",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 740, 680,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (g_hMainWnd == NULL) {
        MessageBoxA(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        delete g_bot;
        return 0;
    }
    
    ShowWindow(g_hMainWnd, SW_SHOW);
    UpdateWindow(g_hMainWnd);
    
    // Start hotkey thread
    CreateThread(NULL, 0, HotkeyThread, g_hMainWnd, 0, NULL);
    
    // Initial log messages
    AddLog("========================================");
    AddLog("   Farming TAI - DedeProjectDev");
    AddLog("   MonsterFinder Bot");
    AddLog("========================================");
    AddLog("Ready to hunt!");
    AddLog("Press F5 to start, F6 to stop, F7 to pause");
    AddLog("Configure settings in monsterfinder_config.ini");
    AddLog("Add monsters in monsters.ini");
    AddLog("========================================");
    
    UpdateStatus("Ready! Configure and press START BOT to begin hunting.");
    
    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
