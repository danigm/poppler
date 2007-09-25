/* Copyright Krzysztof Kowalczyk 2006-2007
   License: GPLv2 */

/* This is a preview support for perf-test for Windows */

#include <windows.h>
#include <assert.h>

#include "SplashBitmap.h"

#define WIN_CLASS_NAME  "PDFTEST_PDF_WIN"
#define COL_WINDOW_BG RGB(0xff, 0xff, 0xff)

static HWND             gHwndSplash;
static HBRUSH           gBrushBg;

static SplashBitmap *gBmpSplash;

int rect_dx(RECT *r)
{
    int dx = r->right - r->left;
    assert(dx >= 0);
    return dx;
}

int rect_dy(RECT *r)
{
    int dy = r->bottom - r->top;
    assert(dy >= 0);
    return dy;
}

static HBITMAP createDIBitmapCommon(SplashBitmap *bmp, HDC hdc)
{
    int bmpDx = bmp->getWidth();
    int bmpDy = bmp->getHeight();
    int bmpRowSize = bmp->getRowSize();

    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(bmih);
    bmih.biHeight = -bmpDy;
    bmih.biWidth = bmpDx;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = bmpDy * bmpRowSize;;
    bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = bmih.biClrImportant = 0;

    unsigned char* bmpData = bmp->getDataPtr();
    HBITMAP hbmp = ::CreateDIBitmap(hdc, &bmih, CBM_INIT, bmpData, (BITMAPINFO *)&bmih , DIB_RGB_COLORS);
    return hbmp;
}

static void stretchDIBitsCommon(SplashBitmap *bmp, HDC hdc, int leftMargin, int topMargin, int pageDx, int pageDy)
{
    int bmpDx = bmp->getWidth();
    int bmpDy = bmp->getHeight();
    int bmpRowSize = bmp->getRowSize();

    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(bmih);
    bmih.biHeight = -bmpDy;
    bmih.biWidth = bmpDx;
    bmih.biPlanes = 1;
    // we could create this dibsection in monochrome
    // if the printer is monochrome, to reduce memory consumption
    // but splash is currently setup to return a full colour bitmap
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = bmpDy * bmpRowSize;;
    bmih.biXPelsPerMeter = bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = bmih.biClrImportant = 0;
    SplashColorPtr bmpData = bmp->getDataPtr();

    ::StretchDIBits(hdc,
        // destination rectangle
        -leftMargin, -topMargin, pageDx, pageDy,
        // source rectangle
        0, 0, bmpDx, bmpDy,
        bmpData,
        (BITMAPINFO *)&bmih ,
        DIB_RGB_COLORS,
        SRCCOPY);
}

/* Set the client area size of the window 'hwnd' to 'dx'/'dy'. */
static void resizeClientArea(HWND hwnd, int x, int dx, int dy, int *dx_out)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    if ((rect_dx(&rc) == dx) && (rect_dy(&rc) == dy))
        return;

    RECT rw;
    GetWindowRect(hwnd, &rw);
    int win_dx = rect_dx(&rw) + (dx - rect_dx(&rc));
    int win_dy = rect_dy(&rw) + (dy - rect_dy(&rc));
    SetWindowPos(hwnd, NULL, x, 0, win_dx, win_dy, SWP_NOACTIVATE | SWP_NOREPOSITION | SWP_NOZORDER);
    if (dx_out)
        *dx_out = win_dx;
}

static void resizeClientAreaToRenderedBitmap(HWND hwnd, SplashBitmap *bmp, int x, int *dxOut)
{
    int dx = bmp->getWidth();
    int dy = bmp->getHeight();
    resizeClientArea(hwnd, x, dx, dy, dxOut);
}

static void drawBitmap(HWND hwnd, SplashBitmap *bmp)
{
    PAINTSTRUCT     ps;

    HDC hdc = BeginPaint(hwnd, &ps);
    SetBkMode(hdc, TRANSPARENT);
    FillRect(hdc, &ps.rcPaint, gBrushBg);

    HBITMAP hbmp = createDIBitmapCommon(bmp, hdc);
    if (hbmp) {
        HDC bmpDC = CreateCompatibleDC(hdc);
        if (bmpDC) {
            SelectObject(bmpDC, hbmp);
            int xSrc = 0, ySrc = 0;
            int xDest = 0, yDest = 0;
            int bmpDx = bmp->getWidth();
            int bmpDy = bmp->getHeight();
            BitBlt(hdc, xDest, yDest, bmpDx, bmpDy, bmpDC, xSrc, ySrc, SRCCOPY);
            DeleteDC(bmpDC);
            bmpDC = NULL;
        }
        DeleteObject(hbmp);
        hbmp = NULL;
    }
    EndPaint(hwnd, &ps);
}

static void onPaint(HWND hwnd)
{
    if (hwnd == gHwndSplash) {
        if (gBmpSplash) {
            drawBitmap(hwnd, gBmpSplash);
        }
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
            // do nothing
            break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_PAINT:
            /* it might happen that we get WM_PAINT after destroying a window */
            onPaint(hwnd);
            break;

        case WM_DESTROY:
            /* WM_DESTROY might be sent as a result of File\Close, in which case CloseWindow() has already been called */
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

static BOOL registerWinClass(void)
{
    WNDCLASSEX  wcex;
    ATOM        atom;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = NULL;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = WIN_CLASS_NAME;
    wcex.hIconSm        = NULL;

    atom = RegisterClassEx(&wcex);
    if (atom)
        return TRUE;
    return FALSE;
}

static bool initWinIfNecessary(void)
{
    if (gHwndSplash)
        return true;

    if (!registerWinClass())
        return false;

    gBrushBg = CreateSolidBrush(COL_WINDOW_BG);

    gHwndSplash = CreateWindow(
        WIN_CLASS_NAME, "Splash",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0,
        NULL, NULL,
        NULL, NULL);

    if (!gHwndSplash)
        return false;

    ShowWindow(gHwndSplash, SW_HIDE);
    return true;
}

static void pumpMessages(void)
{
    BOOL    isMessage;
    MSG     msg;

    for (;;) {
        isMessage = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if (!isMessage)
            return;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void PreviewBitmapInit(void)
{
    /* no need to do anything */
}

void PreviewBitmapDestroy(void)
{
    PostQuitMessage(0);
    pumpMessages();
    DeleteObject(gBrushBg);
}

static void UpdateWindows(void)
{
    if (gBmpSplash) {
        resizeClientAreaToRenderedBitmap(gHwndSplash, gBmpSplash, 0, NULL);
        ShowWindow(gHwndSplash, SW_SHOW);
        InvalidateRect(gHwndSplash, NULL, FALSE);
        UpdateWindow(gHwndSplash);
    } else {
        ShowWindow(gHwndSplash, SW_HIDE);
    }

    pumpMessages();
}

void PreviewBitmapSplash(SplashBitmap *bmpSplash)
{
    if (!initWinIfNecessary())
        return;

    gBmpSplash = bmpSplash;
    UpdateWindows();
}


