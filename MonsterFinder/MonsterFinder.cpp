#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <cstring>

using namespace std;

// ==========================================
//           KONFIGURASI GLOBAL
// ==========================================

const char* WINDOW_NAME = "RF Online";
const int MAX_ATTACK_TIME = 10000; // Max 10 detik menyerang
const int TOLERANCE = 45;

// Resolusi & Scan Area
const int SCAN_WIDTH = 1400;
const int SCAN_HEIGHT = 700;
const int PLAYER_SAFE_RADIUS = 130;

// Setting Motion & HP
const int MOTION_THRESHOLD = 30; // Sensitivitas gerakan (Semakin kecil semakin sensitif)
const int HP_BAR_RED_THRESHOLD = 15;

bool g_autoLoot = true;
bool g_showBoxes = true;
const int AUTO_LOOT_INTERVAL_MS = 2000;

int targetR = 0, targetG = 0, targetB = 0;
bool g_isBotRunning = false;
bool g_isCalibrating = false;
HWND g_hStatusLabel = NULL;
HWND g_hMainWnd = NULL;

// ==========================================
//        STRUKTUR DATA
// ==========================================

struct TargetPos {
    int x, y;
    double dist;
};

struct BoundingBox {
    int minX, maxX, minY, maxY;
    int pixelCount;
    int centerX, centerY;
    double distToCenter;
};

// ==========================================
//     SCANNER (MOTION FILTER)
// ==========================================

class FastScanner {
private:
    HWND hwnd;
    HDC hdcWindow;
    HDC hdcMem;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    BYTE* pBits;
    BYTE* pPrevBits;
    int width, height;
    int bufferSize;

public:
    FastScanner(HWND targetHwnd, int w, int h) {
        hwnd = targetHwnd;
        width = w;
        height = h;
        bufferSize = width * height * 4;

        hdcWindow = GetDC(hwnd);
        hdcMem = CreateCompatibleDC(hdcWindow);

        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
        hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
        pPrevBits = new BYTE[bufferSize];
        memset(pPrevBits, 0, bufferSize);
    }

    void Capture() {
        memcpy(pPrevBits, pBits, bufferSize);

        RECT rect;
        GetClientRect(hwnd, &rect);
        int winW = rect.right - rect.left;
        int winH = rect.bottom - rect.top;
        int startX = (winW / 2) - (width / 2);
        int startY = (winH / 2) - (height / 2);
        if (startX < 0) startX = 0;
        if (startY < 0) startY = 0;

        BitBlt(hdcMem, 0, 0, width, height, hdcWindow, startX, startY, SRCCOPY);
    }

    bool CheckPixelWithMotion(int x, int y, int& r, int& g, int& b) {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;

        int offset = (y * width + x) * 4;
        b = pBits[offset];
        g = pBits[offset + 1];
        r = pBits[offset + 2];

        if (abs(r - targetR) < TOLERANCE &&
            abs(g - targetG) < TOLERANCE &&
            abs(b - targetB) < TOLERANCE) {

            int prevR = pPrevBits[offset + 2];
            int prevG = pPrevBits[offset + 1];
            int prevB = pPrevBits[offset];
            int diff = abs(r - prevR) + abs(g - prevG) + abs(b - prevB);

            // Jika warna cocok DAN bergerak -> True
            if (diff > MOTION_THRESHOLD) return true;
        }
        return false;
    }

    bool CheckPixelRaw(int x, int y, int& r, int& g, int& b) {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        int offset = (y * width + x) * 4;
        b = pBits[offset];
        g = pBits[offset + 1];
        r = pBits[offset + 2];
        return true;
    }

    ~FastScanner() {
        delete[] pPrevBits;
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcWindow);
    }
};

// ==========================================
//     FUNGSI DETEKSI HP (DI ATAS KEPALA)
// ==========================================

bool IsMonsterAlive(FastScanner& scanner, int boxMinX, int boxMaxX, int boxMinY) {
    int r, g, b;
    int redPixelCount = 0;

    // PENTING: Kita cek HP Bar DI ATAS kotak biru.
    // boxMinY adalah bagian paling atas dari warna biru monster.
    // HP bar biasanya ada di y - 80 sampai y - 20 relatif terhadap boxMinY.
    int scanTop = boxMinY - 80;
    int scanBottom = boxMinY - 10;

    if (scanTop < 0) scanTop = 0; // Jangan keluar layar

    for (int y = scanTop; y < scanBottom; y += 2) {
        // Cek lebar HP Bar (sedikit lebih lebar dari box monster)
        for (int x = boxMinX - 20; x < boxMaxX + 20; x += 2) {
            if (scanner.CheckPixelRaw(x, y, r, g, b)) {
                // Cek Merah Pekat (HP Bar)
                if (r > 180 && g < 100 && b < 100) {
                    redPixelCount++;
                }
            }
        }
    }

    // Jika ditemukan cukup banyak piksel merah di atas kepala, monster hidup
    return (redPixelCount > HP_BAR_RED_THRESHOLD);
}

// ==========================================
//     FUNGSI INPUT
// ==========================================

void ForceFocus(HWND hwnd) {
    if (GetForegroundWindow() != hwnd) {
        ShowWindow(hwnd, SW_RESTORE);
        SetForegroundWindow(hwnd);
        Sleep(50);
    }
}

void SendKeyPhysical(WORD key) {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void ClickPhysical(int screenX, int screenY) {
    INPUT inputs[3] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = (screenX * 65535) / GetSystemMetrics(SM_CXSCREEN);
    inputs[0].mi.dy = (screenY * 65535) / GetSystemMetrics(SM_CYSCREEN);
    inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[2].type = INPUT_MOUSE;
    inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(3, inputs, sizeof(INPUT));
}

void UpdateStatus(const char* text) {
    if (g_hStatusLabel) {
        SetWindowTextA(g_hStatusLabel, text);
    }
}

// ==========================================
//     LOGIKA UTAMA (THREAD)
// ==========================================

DWORD WINAPI BotThread(LPVOID lpParam) {
    HWND hwnd = FindWindowA(NULL, WINDOW_NAME);
    if (!hwnd) {
        UpdateStatus("Error: RF Online Window Not Found!");
        return 0;
    }

    FastScanner scanner(hwnd, SCAN_WIDTH, SCAN_HEIGHT);

    bool isAttacking = false;
    DWORD attackStartTime = 0;
    BoundingBox currentTargetBox; // Simpan target yang sedang diserang
    DWORD lastLootTime = GetTickCount();

    UpdateStatus("Bot Started: Moving Object = Monster");

    while (g_isBotRunning) {
        if (!IsWindow(hwnd)) {
            UpdateStatus("Game Window Closed.");
            g_isBotRunning = false;
            break;
        }

        // Auto Loot
        if (g_autoLoot) {
            if (GetTickCount() - lastLootTime > AUTO_LOOT_INTERVAL_MS) {
                SendKeyPhysical('X');
                lastLootTime = GetTickCount();
            }
        }

        scanner.Capture();

        std::vector<TargetPos> rawPixels;
        int r, g, b;

        // 1. SCANNING: Cari Warna + Gerakan
        for (int y = 0; y < SCAN_HEIGHT; y += 3) {
            for (int x = 0; x < SCAN_WIDTH; x += 3) {
                if (scanner.CheckPixelWithMotion(x, y, r, g, b)) {
                    double dist = sqrt(pow(x - SCAN_WIDTH / 2, 2) + pow(y - SCAN_HEIGHT / 2, 2));
                    rawPixels.push_back({ x, y, dist });
                }
            }
        }

        // 2. GROUPING
        std::vector<BoundingBox> allBoxes;
        for (const auto& p : rawPixels) {
            bool merged = false;
            for (auto& box : allBoxes) {
                if (abs(p.x - box.centerX) + abs(p.y - box.centerY) < 60) {
                    if (p.x < box.minX) box.minX = p.x;
                    if (p.x > box.maxX) box.maxX = p.x;
                    if (p.y < box.minY) box.minY = p.y;
                    if (p.y > box.maxY) box.maxY = p.y;
                    box.pixelCount++;
                    box.centerX = (box.minX + box.maxX) / 2;
                    box.centerY = (box.minY + box.maxY) / 2;
                    merged = true;
                    break;
                }
            }
            if (!merged) {
                allBoxes.push_back({ p.x, p.x, p.y, p.y, 1, p.x, p.y, 0 });
            }
        }

        // Filter Boxes (Hapus yang kecil & player)
        std::vector<BoundingBox> validBoxes;
        for (auto& box : allBoxes) {
            if (box.pixelCount < 15) continue; // Min size
            box.distToCenter = sqrt(pow(box.centerX - SCAN_WIDTH / 2, 2) + pow(box.centerY - SCAN_HEIGHT / 2, 2));
            if (box.distToCenter < PLAYER_SAFE_RADIUS) continue; // Ignore Player
            validBoxes.push_back(box);
        }

        // 3. GAMBAR BOX (SELALU DIJALANKAN SETIAP FRAME)
        if (g_showBoxes) {
            HDC hdc = GetDC(hwnd);
            if (hdc) {
                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                HBRUSH hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

                RECT rect; GetClientRect(hwnd, &rect);
                int offsetX = (rect.right - rect.left) / 2 - SCAN_WIDTH / 2;
                int offsetY = (rect.bottom - rect.top) / 2 - SCAN_HEIGHT / 2;

                // Gambar kotak untuk semua monster terdeteksi
                for (const auto& box : validBoxes) {
                    Rectangle(hdc, offsetX + box.minX, offsetY + box.minY, offsetX + box.maxX, offsetY + box.maxY);
                }

                // Jika sedang menyerang, gambar kotak target dengan warna beda (Kuning) agar jelas
                if (isAttacking) {
                    HPEN hPenTarget = CreatePen(PS_SOLID, 3, RGB(255, 255, 0));
                    SelectObject(hdc, hPenTarget);
                    Rectangle(hdc,
                        offsetX + currentTargetBox.minX, offsetY + currentTargetBox.minY,
                        offsetX + currentTargetBox.maxX, offsetY + currentTargetBox.maxY);
                    DeleteObject(hPenTarget);
                }

                SelectObject(hdc, hOldBrush);
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
                ReleaseDC(hwnd, hdc);
            }
        }

        // 4. LOGIC ATTACK
        if (!isAttacking) {
            // Jika IDLE dan ada target bergerak -> LANGSUNG SERANG
            if (!validBoxes.empty()) {
                std::sort(validBoxes.begin(), validBoxes.end(), [](const BoundingBox& a, const BoundingBox& b) {
                    return a.distToCenter < b.distToCenter;
                    });

                currentTargetBox = validBoxes[0]; // Ambil yang terdekat
                isAttacking = true;
                attackStartTime = GetTickCount();

                UpdateStatus("Moving Target Found! Attacking...");
                ForceFocus(hwnd);

                RECT rect; GetClientRect(hwnd, &rect);
                int offsetX = (rect.right - rect.left) / 2 - SCAN_WIDTH / 2;
                int offsetY = (rect.bottom - rect.top) / 2 - SCAN_HEIGHT / 2;

                int clickX = offsetX + currentTargetBox.centerX;
                int clickY = offsetY + currentTargetBox.centerY;

                POINT pt = { clickX, clickY };
                ClientToScreen(hwnd, &pt);

                ClickPhysical(pt.x, pt.y);
                Sleep(100); // Tunggu sedikit agar game respond
                SendKeyPhysical(VK_SPACE); // Langsung Skill/Attack

            }
            else {
                static DWORD counter = 0;
                if (++counter % 50 == 0) UpdateStatus("Scanning for moving objects...");
            }
        }
        else {
            // SEDANG ATTACK: CEK HP BAR DI ATAS KEPALA
            // Kita beri delay kecil saat awal attack (300ms) sebelum cek HP, agar UI game sempat update
            if (GetTickCount() - attackStartTime < 300) {
                UpdateStatus("Initializing Attack...");
            }
            else {
                bool hpExists = IsMonsterAlive(scanner, currentTargetBox.minX, currentTargetBox.maxX, currentTargetBox.minY);

                if (hpExists) {
                    // Masih Hidup
                    if (GetTickCount() - attackStartTime > MAX_ATTACK_TIME) {
                        UpdateStatus("Timeout! Skipping...");
                        SendKeyPhysical(VK_ESCAPE);
                        isAttacking = false;
                    }
                    else {
                        UpdateStatus("Attacking! (HP Detected)");
                    }
                }
                else {
                    // Mati (HP Bar hilang dari atas kepala)
                    UpdateStatus("Target Dead!");
                    isAttacking = false; // Kembali scanning
                }
            }
        }
        Sleep(10);
    }

    UpdateStatus("Bot Stopped.");
    return 0;
}

// ==========================================
//     FUNGSI KALIBRASI
// ==========================================

void StartCalibration() {
    g_isCalibrating = true;
    UpdateStatus("Klik Kiri pada monster biru.");

    MSG msg;
    HDC hdc = GetDC(NULL);
    bool done = false;

    while (!done && g_isCalibrating) {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
            POINT p;
            GetCursorPos(&p);
            COLORREF color = GetPixel(hdc, p.x, p.y);
            targetR = GetRValue(color);
            targetG = GetGValue(color);
            targetB = GetBValue(color);

            char buf[100];
            sprintf_s(buf, sizeof(buf), "Locked! R:%d G:%d B:%d", targetR, targetG, targetB);
            UpdateStatus(buf);
            Beep(1000, 200);
            done = true;
        }

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(20);
    }
    ReleaseDC(NULL, hdc);
    g_isCalibrating = false;
}

// ==========================================
//           GUI WINDOW PROCEDURE
// ==========================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        CreateWindowA("STATIC", "RF Hunter (Motion = Attack)", WS_VISIBLE | WS_CHILD | SS_CENTER,
            20, 10, 340, 25, hwnd, NULL, NULL, NULL);

        g_hStatusLabel = CreateWindowA("STATIC", "Status: Ready", WS_VISIBLE | WS_CHILD | SS_LEFT,
            20, 50, 340, 20, hwnd, NULL, NULL, NULL);

        CreateWindowA("BUTTON", "1. Set Monster Color", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 90, 340, 40, hwnd, (HMENU)1, NULL, NULL);

        CreateWindowA("BUTTON", "2. Start Bot", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 150, 160, 40, hwnd, (HMENU)2, NULL, NULL);

        CreateWindowA("BUTTON", "3. Stop Bot", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            200, 150, 160, 40, hwnd, (HMENU)3, NULL, NULL);

        CreateWindowA("BUTTON", "Exit", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 210, 340, 30, hwnd, (HMENU)4, NULL, NULL);
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case 1:
            if (!g_isBotRunning) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartCalibration, NULL, 0, NULL);
            else MessageBoxA(hwnd, "Stop bot dulu!", "Info", MB_OK);
            break;
        case 2:
            if (!g_isBotRunning) {
                g_isBotRunning = true;
                CreateThread(NULL, 0, BotThread, NULL, 0, NULL);
            }
            break;
        case 3:
            g_isBotRunning = false;
            UpdateStatus("Bot Stopped.");
            break;
        case 4:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    case WM_DESTROY:
        g_isBotRunning = false;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int main() {
    const char* CLASS_NAME = "RF_BOT_MOTION_ATTACK";

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassA(&wc)) {
        MessageBoxA(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    g_hMainWnd = CreateWindowExA(
        0, CLASS_NAME, "RF Hunter - Aggressive Motion",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (g_hMainWnd == NULL) {
        MessageBoxA(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(g_hMainWnd, SW_SHOW);
    UpdateWindow(g_hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}