//
// g++ slwa-test.cc -o slwa-test -mwindows -lgdiplus -ldwmapi
//

#define  _WIN32_WINNT 0x0600

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>

int width = 360;
int height = 360;
HBITMAP hBitmap;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
    case WM_PAINT:
      {
        static unsigned char i = 0;
        i++;
        SetLayeredWindowAttributes(hWnd, RGB(0,0,0), 255 - (i % 4), LWA_ALPHA);

        PAINTSTRUCT ps;
        HDC hdcUpdate = BeginPaint(hWnd, &ps);

        HDC hdcMem = CreateCompatibleDC(hdcUpdate);
        HBITMAP hbmpold = (HBITMAP)SelectObject(hdcMem, hBitmap);

#if 0
        if (!BitBlt(hdcUpdate, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hdcMem, 0, 0, SRCCOPY))
          {
            printf("BitBlt failed: 0x%08x\n", (int)GetLastError());
          }
#endif

        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 255;
        bf.AlphaFormat = AC_SRC_ALPHA;
        if (!GdiAlphaBlend(hdcUpdate, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hdcMem, 0, 0, width, height, bf))
          {
            printf("AlphaBlend failed: 0x%08x\n", (int)GetLastError());
          }

        SelectObject(hdcMem, hbmpold);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
      }
      return 0;

    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

  LPCTSTR szWindowClass = "TransparentClass";

  // Register class
  WNDCLASSEX wcex = {0};
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wcex.lpfnWndProc    = WndProc;
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hInstance      = hInstance;
  wcex.hIcon          = NULL;
  wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm        = NULL;
  wcex.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  RegisterClassEx(&wcex);

  // Create window
  HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
                             szWindowClass,
                             "Transparent Window",
                             WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                             NULL, NULL, hInstance, NULL);

  Gdiplus::Bitmap *m_pImage = Gdiplus::Bitmap::FromFile(L"sample.png", FALSE);
  Gdiplus::Color bg(0,0,0,0);
  m_pImage->GetHBITMAP(bg, &hBitmap);
  assert(hBitmap);

  SetLayeredWindowAttributes(hWnd, RGB(0,0,0), 0, LWA_ALPHA);

  DWM_BLURBEHIND blurBehind = { 0 };
  blurBehind.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
  blurBehind.hRgnBlur = CreateRectRgn(-1, -1, 0, 0);
  blurBehind.fEnable = TRUE;
  blurBehind.fTransitionOnMaximized = FALSE;
  DwmEnableBlurBehindWindow(hWnd, &blurBehind);
  DeleteObject(blurBehind.hRgnBlur);

  ShowWindow(hWnd, SW_SHOW);

  // Main message loop
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

  return (int)msg.wParam;
}
