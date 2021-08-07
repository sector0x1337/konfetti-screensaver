#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

char *clsName = "konfetti-1.0.0";
int xmax, ymax;
POINT maus;

enum runtype { SCR_PREV, SCR_CONFIG, SCR_RUN, SCR_ERRPREV };

LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);

enum runtype getRunType(LPSTR args, HWND *hWnd) {
    long long res;

    if (strlen(args)<2) {
        return SCR_RUN;
    }
    if (*args!='/') {
        return SCR_CONFIG;
    }
    args++;
    switch (*args) {
        case 'c':
        case 'C':
            return SCR_CONFIG;
        
        case 'p':
        case 'P':
            args++;
            if (strlen(args)==0) {
                return SCR_ERRPREV;
            }
            res=0;
            while (*args==' ') {
                args++;
            }
            while (*args!='\0') {
                char c=*args;
                int ziff;
                args++;
                if (c<'0' || c>'9') {
                    return SCR_ERRPREV;
                }
                ziff=c-'0';
                res=10*res+ziff;
            }
            *hWnd=(HWND)res;
            return SCR_PREV;
    }

    return SCR_RUN;
}

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int show) {
    MSG messages;
    WNDCLASSEX wndcl;
    HWND hWnd;

    wndcl.cbClsExtra=0;
    wndcl.cbSize=sizeof(WNDCLASSEX);
    wndcl.cbWndExtra=0;
    wndcl.hbrBackground=(HBRUSH)CreateSolidBrush(RGB(0,0,0));
    wndcl.hCursor=NULL;
    wndcl.hIcon=LoadIcon(NULL, IDI_APPLICATION);
    wndcl.hIconSm=LoadIcon(NULL, IDI_APPLICATION);
    wndcl.hInstance=hThisInstance;
    wndcl.lpfnWndProc=wndProc;
    wndcl.lpszClassName=clsName;
    wndcl.lpszMenuName=NULL;
    wndcl.style=0;

    srand(time(NULL));

    switch (getRunType(szCmdLine, &hWnd)) {
        case SCR_RUN:
            xmax = GetSystemMetrics(SM_CXSCREEN);
            ymax = GetSystemMetrics(SM_CYSCREEN);

            if (xmax==0 || ymax==0) {
                MessageBox(NULL, "Konnte Bildschirmdimension nicht abfragen.", "GetSystemMetrics", MB_ICONERROR|MB_OK);
                return -1;
            }

            if (RegisterClassEx(&wndcl)==0) {
                MessageBox(NULL, "Konnte Fensterklasse nicht registrieren.", "RegisterClassEx", MB_ICONERROR|MB_OK);
                return -1;
            }

            hWnd=CreateWindowEx(
                WS_EX_APPWINDOW, clsName, NULL, WS_VISIBLE, 0, 0,
                xmax, ymax, HWND_DESKTOP, NULL, hThisInstance,  NULL
            );
            if (hWnd==NULL) {
                MessageBox(NULL, "Fenster konnte nicht erstellt werden.", "CreateWindowEx", MB_ICONERROR|MB_OK);
                return -1;
            }

            if (GetCursorPos(&maus)==0) {
                MessageBox(NULL, "Kursorposition konnte nicht ermittelt werden.", "GetCursorPos",MB_ICONERROR|MB_OK);
                return -1;
            }
            if (ShowWindow(hWnd, show)==0) {
                MessageBox(NULL, "Bildschirmschoner konnte nicht hergestellt werden", "ShowWindow", MB_ICONERROR|MB_OK);
                return -1;
            }
            break;

        case SCR_PREV: {
            return 0; // Vorschau bisher nicht implementiert.
        }
        case SCR_CONFIG:
            MessageBox(0,"Dieser Bildschirmschoner ist nicht konfigurierbar.",0 , 0);
            return 1;

        case SCR_ERRPREV:
            MessageBox(0,"Ungültige Parameter.", 0, 0);
            break;
    }

    while (GetMessage(&messages, NULL, 0, 0)) {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return 0;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    RECT rct;

    switch (msg) {
        case WM_CREATE: {
            UINT_PTR tid=1;
            DWORD dw;

            dw=GetWindowLong(hWnd, GWL_STYLE);
            dw=dw&(~(WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX));
            SetWindowLong(hWnd, GWL_STYLE, dw);
            SetTimer(hWnd, tid, 70, NULL);
            break;
        }

        case WM_TIMER: {
            HDC hdc;
            HBRUSH col;
            int x, y;
            unsigned p=0;

            hdc=GetDC(hWnd);
            col=CreateSolidBrush(RGB(rand()%256, rand()%256, rand()%256));
            if (hdc) {
                p++;
            }
            if (col) {
                p++;
            }
            x=rand()%xmax;
            y=rand()%ymax;
            rct.top=y;
            rct.left=x;
            rct.right=x+10;
            rct.bottom=y+10;
            if (FillRect(hdc, &rct, col)) {
                p++;
            }

            if (p<3) {
                PostQuitMessage(-1); // Fehler, Konnte nicht zeichnen.
            }
            DeleteObject(col);
            ReleaseDC(hWnd, hdc);
            break;
        }

        case WM_CLOSE:
            PostQuitMessage(0);
            break;


        //DefScreenSaverProc:
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_NCACTIVATE:
            if (wParam==FALSE) {
                SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
            break;

        case WM_SETCURSOR:
            SetCursor(NULL);
            return TRUE;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_KEYUP:
            PostQuitMessage(0);
            break;

        case WM_MOUSEMOVE: {
            DWORD lmp=(maus.y<<16)|maus.x;
            if (lmp!=lParam) {
                PostQuitMessage(0);
            }
            return 0;
        }

        case WM_DESTROY:
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case WM_SYSCOMMAND:
            if (wParam==SC_CLOSE || wParam==SC_SCREENSAVE) {
                return FALSE;
            }
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}