//
// g++ ulw-test.cc -o ulw-test -mwindows -lgdiplus
//

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>
#include <dwmapi.h>

// Window message handler
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
    {
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
  wcex.style          = CS_HREDRAW | CS_VREDRAW;
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

  int width = 360;
  int height = 360;

  HWND hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_OVERLAPPEDWINDOW | WS_EX_APPWINDOW,
                             szWindowClass,
                             "Transparent Window",
                             WS_OVERLAPPED | WS_SYSMENU,
                             CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                             NULL, NULL, hInstance, NULL);


  HBITMAP hBitmap;
  Gdiplus::Bitmap *m_pImage = Gdiplus::Bitmap::FromFile(L"sample.png", FALSE);
  Gdiplus::Color bg(0,0,0,0);
  m_pImage->GetHBITMAP(bg, &hBitmap);
  assert(hBitmap);

  HDC hdcScreen = GetDC(0);
  HDC hdc = CreateCompatibleDC(hdcScreen);
  ReleaseDC(0, hdcScreen);
  HBITMAP hbmpold = (HBITMAP)SelectObject(hdc, hBitmap);

  POINT dcOffset = {0, 0};
  SIZE size = {width, height};
  BLENDFUNCTION bf;
  bf.BlendOp = AC_SRC_OVER;
  bf.BlendFlags = 0;
  bf.SourceConstantAlpha = 255;
  bf.AlphaFormat = AC_SRC_ALPHA;
  if (!UpdateLayeredWindow(hWnd, 0, 0, &size, hdc, &dcOffset, 0, &bf, ULW_ALPHA))
    printf("ULW failed\n");

  SelectObject(hdc, hbmpold);
  DeleteDC(hdc);
  DeleteObject(hBitmap);

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
