#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <CommCtrl.h>
#include "resource.h"

#define DEBUG
#ifdef DEBUG
    #include <conio.h>
#endif // DEBUG

#define bt(A) bt_##A
#define numIfCom(A) if((HWND)lParam == bt(A)) {num = num * 10.0 + (double)(A);}
#define NUMIFCOM numIfCom(0); numIfCom(1); numIfCom(2); numIfCom(3); numIfCom(4);\
                 numIfCom(5); numIfCom(6); numIfCom(7); numIfCom(8); numIfCom(9)

#define nIfCom(A) if((HWND)lParam == bt(A)) {littleNum += (double)(A) / pow(10.0, countRazr);\
                                countRazr++;}
#define LITTLENUMIFCOM nIfCom(0); nIfCom(1); nIfCom(2); nIfCom(3); nIfCom(4);\
                       nIfCom(5); nIfCom(6); nIfCom(7); nIfCom(8); nIfCom(9)

#define numIfKey(B) if( (wParam == 0x30 + (B)) || (wParam == 0x60 + (B)) ) {num = num * 10.0 + (double)B;}
#define NUMIFKEY numIfKey(0); numIfKey(1); numIfKey(2); numIfKey(3); numIfKey(4); numIfKey(5);\
                 numIfKey(6); numIfKey(7); numIfKey(8); numIfKey(9)

#define nIfKey(B) if( (wParam == 0x30 + (B)) || (wParam == 0x60 + (B)) ) {littleNum += (double)(B) / pow(10.0, countRazr);\
                                countRazr++;}
#define LITTLENUMIFKEY nIfKey(0); nIfKey(1); nIfKey(2); nIfKey(3); nIfKey(4);\
                       nIfKey(5); nIfKey(6); nIfKey(7); nIfKey(8); nIfKey(9)

#define MAXNUMSIZE 200 // четные элементы для хранения чисел, нечетные для математического действия (200 это 100 математических действий подряд)
                       // even elements for storing numbers, odd for mathematical operations (200 is 100 mathematical operations in a row)

#define CLR_WND_BCG RGB(47,248,222)
#define CLR_COLOR_BCG RGB(255, 255, 255)

#define id_hSystemMenu_Color 100

HWND hWndCalc, outputScreen, inputScreen, hColor;

HWND bt_C, bt_back, bt_percent, bt_division, bt_7, bt_8, bt_9, bt_multi,
     bt_4, bt_5, bt_6, bt_minus, bt_1, bt_2, bt_3, bt_plus,
     bt_plusAndMinus, bt_0, bt_point, bt_equals;

HWND hTrackbarRed, hTrackbarGreen, hTrackbarBlue;

HMENU hSystemMenu;

WINDOWPLACEMENT wp;
int Red = 220, Green = 220, Blue = 220;
FILE *f = NULL;

PAINTSTRUCT psPaint;
HDC hdcPaint;

HBRUSH hbrBkgTrack;

double result = 0;
double masNum[MAXNUMSIZE] = {};
double num = 0;
double littleNum = 0;
double countRazr = 1;
int countMasNum = 0;
double precision = 9;

char strInputScreen[50] = {};
char strNum[25] = {};
char strOutputScreen[MAXNUMSIZE * 20] = {};
char masMath[][10] = {" + ", " - ", " / ", " * "};

char strSpecificator[10] = {};
char strNumSpecif[3] = {};

enum {plus, minus, division, multi} math;

BOOL flagMinus = FALSE;
BOOL flagPoint = FALSE;
BOOL flagDivideZero = FALSE;
BOOL flagBtBack = FALSE;
BOOL flagShowColor = FALSE;

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef double (*TFunc)(double, double);
double funcPlus(double a, double b);
double funcMinus(double a, double b);
double funcDivision(double a, double b);
double funcMulti(double a, double b);
TFunc Func[4] = {funcPlus, funcMinus, funcDivision, funcMulti};
double funcMath(double num1, double num2, TFunc fn);

void ShowOutputScreen();
void ShowNumMasInputScreen();
void CalcAndShowResultInputScreen();

char* makeSpecificator(double);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
    f = fopen("posWnd.txt", "rb");
    if (f != NULL)
    {
        fread(&wp, sizeof(wp), 1, f);
        fread(&Red, sizeof(int), 1, f);
        fread(&Green, sizeof(int), 1, f);
        fread(&Blue, sizeof(int), 1, f);
    }
    #ifdef DEBUG
        wprintf(L"main wp: %d %d %d %d %d %d\n", wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.ptMinPosition.x,\
                wp.ptMinPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.top);
        wprintf(L"Red %d, Green %d, Blue %d\n", Red, Green, Blue);
    #endif // DEBUG

    const char CLASS_NAME[] = "My Window";
    const char WINDOW_NAME[] = "Calculator";

    WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = CreateSolidBrush(RGB(Red, Green,Blue));
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

    RegisterClass(&wc);

    hWndCalc = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 360, 540, NULL, NULL, NULL, NULL); // 360 540

    hSystemMenu = GetSystemMenu(hWndCalc, FALSE);

    MENUITEMINFO mColor = {};
        mColor.cbSize = sizeof(MENUITEMINFO);
        mColor.fMask = MIIM_FTYPE | MIIM_STRING | MIIM_ID;
        mColor.fType = MFT_STRING;
        mColor.dwTypeData = "Color";
        mColor.cch = 10;
        mColor.wID = id_hSystemMenu_Color;

    InsertMenuItem(hSystemMenu, 0, TRUE, &mColor);

    outputScreen = CreateWindow("static", "0", WS_VISIBLE | WS_CHILD | ES_RIGHT | WS_BORDER, 10, 20, 335, 100, hWndCalc, NULL, NULL, NULL);
    inputScreen = CreateWindow("static", "0", WS_VISIBLE | WS_CHILD | ES_RIGHT | WS_BORDER, 10, 130, 335, 50, hWndCalc, NULL, NULL, NULL);

    bt_C = CreateWindow("button", "C", WS_VISIBLE | WS_CHILD, 10, 200, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_back = CreateWindow("button", "back", WS_VISIBLE | WS_CHILD, 95, 200, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_percent = CreateWindow("button", "%", WS_VISIBLE | WS_CHILD, 180, 200, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_division = CreateWindow("button", "/", WS_VISIBLE | WS_CHILD, 265, 200, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_7 = CreateWindow("button", "7", WS_VISIBLE | WS_CHILD, 10, 260, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_8 = CreateWindow("button", "8", WS_VISIBLE | WS_CHILD, 95, 260, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_9 = CreateWindow("button", "9", WS_VISIBLE | WS_CHILD, 180, 260, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_multi = CreateWindow("button", "X", WS_VISIBLE | WS_CHILD, 265, 260, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_4 = CreateWindow("button", "4", WS_VISIBLE | WS_CHILD, 10, 320, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_5 = CreateWindow("button", "5", WS_VISIBLE | WS_CHILD, 95, 320, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_6 = CreateWindow("button", "6", WS_VISIBLE | WS_CHILD, 180, 320, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_minus = CreateWindow("button", "-", WS_VISIBLE | WS_CHILD, 265, 320, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_1 = CreateWindow("button", "1", WS_VISIBLE | WS_CHILD, 10, 380, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_2 = CreateWindow("button", "2", WS_VISIBLE | WS_CHILD, 95, 380, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_3 = CreateWindow("button", "3", WS_VISIBLE | WS_CHILD, 180, 380, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_plus = CreateWindow("button", "+", WS_VISIBLE | WS_CHILD, 265, 380, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_plusAndMinus = CreateWindow("button", "+/-", WS_VISIBLE | WS_CHILD, 10, 440, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_0 = CreateWindow("button", "0", WS_VISIBLE | WS_CHILD, 95, 440, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_point = CreateWindow("button", ",", WS_VISIBLE | WS_CHILD, 180, 440, 80, 50, hWndCalc, NULL, NULL, NULL);
    bt_equals = CreateWindow("button", "=", WS_VISIBLE | WS_CHILD, 265, 440, 80, 50, hWndCalc, NULL, NULL, NULL);

    hColor = CreateWindow(CLASS_NAME, "Color", WS_DISABLED | WS_POPUP | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 300, 280,
                          hWndCalc, NULL, NULL, NULL);

    hTrackbarRed = CreateWindow(TRACKBAR_CLASS, "TrackBar", WS_VISIBLE | WS_CHILD,  50, 30, 200, 30,
                          hColor, NULL, NULL, NULL);
    hTrackbarGreen = CreateWindow(TRACKBAR_CLASS, "TrackBar", WS_VISIBLE | WS_CHILD, 50, 110, 200, 30,
                          hColor, NULL, NULL, NULL);
    hTrackbarBlue = CreateWindow(TRACKBAR_CLASS, "TrackBar", WS_VISIBLE | WS_CHILD, 50, 190, 200, 30,
                          hColor, NULL, NULL, NULL);

    SendMessage(hTrackbarRed, TBM_SETRANGE, TRUE, MAKELONG(0,255));
    SendMessage(hTrackbarRed, TBM_SETPOS, TRUE, Red);

    SendMessage(hTrackbarGreen, TBM_SETRANGE, TRUE, MAKELONG(0,255));
    SendMessage(hTrackbarGreen, TBM_SETPOS, TRUE, Green);

    SendMessage(hTrackbarBlue, TBM_SETRANGE, TRUE, MAKELONG(0,255));
    SendMessage(hTrackbarBlue, TBM_SETPOS, TRUE, Blue);

    ShowWindow(hWndCalc, SW_NORMAL);

    if(f != NULL)
    {
        wp.length = sizeof(wp);
        SetWindowPlacement(hWndCalc, &wp);
    }
    fclose(f);

    MSG msg = {};

    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_COMMAND:
        case WM_KEYDOWN:

        #ifdef DEBUG
            printf("WM_COMMAND WM_KEYDOWN\n");
        #endif // DEBUG

            if ((HWND)lParam == bt_C || wParam == VK_ESCAPE)
            {
                num = littleNum = 0;
                countRazr = 1;
                countMasNum = 0;
                memset(masNum, 0, sizeof(masNum));
                flagMinus = FALSE;
                flagPoint = FALSE;
                flagDivideZero = FALSE;
            }

            if ((HWND)lParam == bt_plusAndMinus)
            {
                if(flagMinus)
                    flagMinus = FALSE;
                else
                    flagMinus = TRUE;
            }

            if ((HWND)lParam == bt_point || wParam == VK_DECIMAL)
            {
                flagPoint = TRUE;
            }

            if (countRazr != precision + 1)
            {
                if(flagPoint)
                {
                    LITTLENUMIFCOM;
                    LITTLENUMIFKEY;
                    #ifdef DEBUG
                        printf("littleNum = %g, countRazr = %f\n", littleNum, countRazr);
                    #endif // DEBUG
                }
                else
                {
                    NUMIFCOM;
                    NUMIFKEY;
                    #ifdef DEBUG
                        printf("num = %g, countRazr = %f\n", num, countRazr);
                    #endif // DEBUG
                }
            }

            if(flagMinus)
                masNum[countMasNum] = - num - littleNum;
            else
                masNum[countMasNum] = num + littleNum;
            #ifdef DEBUG
                printf("FlagPoint = %d\n", flagPoint);
                printf("masNum[i] = %g\n", masNum[countMasNum]);
            #endif // DEBUG

            ShowNumMasInputScreen();

            if((HWND)lParam == bt_percent && countMasNum >=2)
            {
                /* расчет суммы элементов до элемента в котором будут указаны проценты */
                /* calculation of the sum of elements up to the element in which percentages will be indicated */
                if(countMasNum >= 4)
                {
                    double res = 0;
                    countMasNum -= 2;
                    for(int i = 0; i < countMasNum; i += 2)
                    {
                        if (i == 0)
                            res = funcMath(masNum[i], masNum[i+2], Func[(int)masNum[i+1]] );
                        else
                            res = funcMath(res, masNum[i+2], Func[(int)masNum[i+1]]);
                    }
                    countMasNum += 2;
                    /* расчет вычисления по всем элементам с учетом расчета последнего элемента в процентах*/
                    /* calculation of calculations for all elements, taking into account the calculation of the last element as a percentage */
                    masNum[countMasNum] = res / 100.0 * masNum[countMasNum];
                    num = masNum[countMasNum];
                    littleNum = 0;
                    #ifdef DEBUG
                        printf("masNum = %g\n", masNum[countMasNum]);
                    #endif // DEBUG
                    CalcAndShowResultInputScreen();
                }
                else
                {
                    /* расчет вычисления по всем элементам с учетам расчета последнего элемента в процентах*/
                    /* calculation of calculations for all elements, taking into account the calculation of the last element as a percentage */
                    masNum[countMasNum] = masNum[countMasNum-2] / 100.0 * masNum[countMasNum];
                    num = masNum[countMasNum];
                    littleNum = 0;
                    #ifdef DEBUG
                        printf("masNum = %g\n", masNum[countMasNum]);
                    #endif // DEBUG
                    CalcAndShowResultInputScreen();
                }
            }

            if ((HWND)lParam == bt_plus || (HWND)lParam == bt_minus || (HWND)lParam == bt_multi || (HWND)lParam == bt_division ||
                      wParam == VK_ADD  ||    wParam == VK_SUBTRACT ||    wParam == VK_MULTIPLY || wParam == VK_DIVIDE)
            {
                flagMinus = FALSE;
                flagPoint = FALSE;
                countMasNum++;
                num = littleNum = 0;
                countRazr = 1;
                SetWindowText(inputScreen, "0");
                masNum[countMasNum] = (HWND)lParam == bt_plus  ? (double)plus :
                                      (HWND)lParam == bt_minus ? (double)minus :
                                      (HWND)lParam == bt_multi ? (double)multi :
                                      wParam == VK_ADD      ? (double)plus :
                                      wParam == VK_SUBTRACT ? (double)minus :
                                      wParam == VK_MULTIPLY ? (double)multi : (double)division;
                #ifdef DEBUG
                    printf("masNum = %f\n", masNum[countMasNum]);
                #endif // DEBUG
                countMasNum++;
            }

            if ( ((HWND)lParam == bt_equals || wParam == VK_RETURN ) && countMasNum > 0)
            {
                flagDivideZero = FALSE;
                CalcAndShowResultInputScreen();
            }

            if ((HWND)lParam == bt_back || wParam == VK_BACK)
            {
                if (countMasNum >= 2)
                {
                    flagMinus = FALSE;
                    flagDivideZero = FALSE;
                    masNum[countMasNum] = 0;
                    countMasNum -= 2;
                    littleNum = 0;
                    num = masNum[countMasNum];
                    flagPoint = FALSE;
                    countRazr = 1;

                    flagBtBack = TRUE;
                    ShowNumMasInputScreen();
                    flagBtBack = FALSE;
                }
            }

            #ifdef DEBUG
                printf("countMasNum = %d\n", countMasNum);
            #endif // DEBUG

            ShowOutputScreen();

            SetFocus(hWnd);

            break;

        case WM_SYSCOMMAND:
            switch (wParam)
            {
                case id_hSystemMenu_Color:
                    EnableWindow(hColor, TRUE);
                    ShowWindow(hColor, SW_NORMAL);
                    flagShowColor = TRUE;
                    break;

                case SC_CLOSE:
                    if (flagShowColor)
                    {
                        EnableWindow(hColor, FALSE);
                        ShowWindow(hColor, SW_HIDE);
                        flagShowColor = FALSE;
                    }
                    else
                        return DefWindowProcA(hWnd, uMsg, wParam, lParam);

                    break;

                default:
                    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
            }
            break;

        case WM_HSCROLL:
            #ifdef DEBUG
                printf("WM_HSCROLL\n");
            #endif // DEBUG

            Red = SendMessage(hTrackbarRed, TBM_GETPOS, 0, 0);
            Green = SendMessage(hTrackbarGreen, TBM_GETPOS, 0, 0);
            Blue = SendMessage(hTrackbarBlue, TBM_GETPOS, 0, 0);

            RedrawWindow(hWndCalc, NULL, NULL, RDW_UPDATENOW | RDW_INVALIDATE);
            Sleep(150);

            #ifdef DEBUG
                printf("Red %d Green %d Blue %d\n", Red, Green, Blue);
            #endif // DEBUG
            break;

        case WM_PAINT:
            #ifdef DEBUG
                printf("WM_PAINT\n");
            #endif // DEBUG

            hdcPaint = BeginPaint(hWndCalc, &psPaint);
                FillRect(hdcPaint, &psPaint.rcPaint, CreateSolidBrush(RGB(Red, Green, Blue)));
            EndPaint(hWndCalc, &psPaint);

            hdcPaint = BeginPaint(hColor, &psPaint);
                FillRect(hdcPaint, &psPaint.rcPaint, CreateSolidBrush(CLR_COLOR_BCG));
            EndPaint(hColor, &psPaint);

            break;

        case WM_CTLCOLORSTATIC:
            #ifdef DEBUG
                printf("WM_CTLCOLORSTATIC\n");
            #endif // DEBUG

            if ((HWND)lParam == hTrackbarRed)
            {
                hbrBkgTrack = CreateSolidBrush(RGB(255,0,0));
                return (INT_PTR)hbrBkgTrack;
            }
            if ((HWND)lParam == hTrackbarGreen)
            {
                hbrBkgTrack = CreateSolidBrush(RGB(0,255,0));
                return (INT_PTR)hbrBkgTrack;
            }
            if ((HWND)lParam == hTrackbarBlue)
            {
                hbrBkgTrack = CreateSolidBrush(RGB(0,0,255));
                return (INT_PTR)hbrBkgTrack;
            }

            return FALSE;

            break;

        case WM_DESTROY:
            wp.length = sizeof(wp);
            GetWindowPlacement(hWndCalc, &wp);
            f = fopen("posWnd.txt", "wb");
            if (f)
            {
                fwrite(&wp, sizeof(wp), 1, f);
                fwrite(&Red, sizeof(int), 1, f);
                fwrite(&Green, sizeof(int), 1, f);
                fwrite(&Blue, sizeof(int), 1, f);
            }
            fclose(f);
            #ifdef DEBUG
                wprintf(L"destroy wp: %d %d %d %d %d %d\n", wp.ptMaxPosition.x, wp.ptMaxPosition.y, wp.ptMinPosition.x,\
                    wp.ptMinPosition.y, wp.rcNormalPosition.left, wp.rcNormalPosition.top);
                wprintf(L"Red %d, Green %d, Blue %d\n", Red, Green, Blue);
                getch();
            #endif // DEBUG
            DeleteMenu(hSystemMenu, 0, MF_BYCOMMAND);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA (hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

char* makeSpecificator(double a)
{
    sprintf(strSpecificator, "%s" , "%.");

    char strNumDouble[30] = {};

        char strSpecPrec[10] = "%.";
        char strNumSpecPrec[3] = {};
        sprintf(strNumSpecPrec, "%d", (int)precision); // задание точности - количество знаков после запятой
                                                       // precision assignment - number of digits after the decimal point
        strcat(strSpecPrec, strNumSpecPrec);
        strcat(strSpecPrec, "f");

    sprintf(strNumDouble, strSpecPrec, a);
    #ifdef DEBUG
        printf("strNumDouble %s\n", strNumDouble);
    #endif // DEBUG
    int razr = 0;
    int numPoint = 0;
    char strAfterProcessing[30] = {};

    for (int i = 0; i < 30; i++)
    {
        if(strNumDouble[i] == '.')
        {
            numPoint = i;
            break;
        }
    }

    sprintf(strAfterProcessing, "%s", &strNumDouble[numPoint]);
    #ifdef DEBUG
        printf("strAfterProcessing %s\n", strAfterProcessing);
    #endif // DEBUG

    for (int i = 29; i >=0 ; i--)
    {
        if (strAfterProcessing[i] > '0' && strAfterProcessing[i] <= '9')
        {
            razr = i;
            break;
        }
    }

    sprintf(strNumSpecif, "%d", razr);
    strcat(strSpecificator, strNumSpecif);
    strcat(strSpecificator, "f");

    return strSpecificator;
}

void ShowOutputScreen()
{
    for (int i = 0; i <= countMasNum; i++)
    {
        if (i%2 == 0)
        {
            snprintf(strNum, sizeof(strNum), makeSpecificator(masNum[i]), masNum[i]);
            strcat(strOutputScreen, strNum);
        }
        else
        {
            strcat(strOutputScreen, masMath[(int)masNum[i]]);
            #ifdef DEBUG
                printf("show = %s\n", masMath[(int)masNum[i]]);
            #endif // DEBUG
        }
    }

    SetWindowText(outputScreen, strOutputScreen);
    memset(strOutputScreen, 0, sizeof(strOutputScreen));
}

double funcPlus(double a, double b)
{
    return a + b;
}

double funcMinus(double a, double b)
{
    return a - b;
}

double funcDivision(double a, double b)
{
    if(b != 0)
        return a / b;
    else
    {
        flagDivideZero = TRUE;
        return a / b;
    }
}

double funcMulti(double a, double b)
{
    return a * b;
}

double funcMath(double num1, double num2, TFunc fn)
{
    return fn(num1, num2);
}

void CalcAndShowResultInputScreen()
{
    for(int i = 0; i < countMasNum; i += 2)
    {
        #ifdef DEBUG
            for(int j = 0; j <= countMasNum; j++)
            {
                printf("mas[%d] = %.5f ", j, masNum[j]);
            }
            printf("\n");
        #endif // DEBUG

        if (i == 0)
            result = funcMath(masNum[i], masNum[i+2], Func[(int)masNum[i+1]] );
        else
            result = funcMath(result, masNum[i+2], Func[(int)masNum[i+1]]);
        #ifdef DEBUG
            printf("result = %0.5f\n", result);
        #endif // DEBUG
    }
    snprintf(strNum, sizeof(strNum), makeSpecificator(result), result);
    #ifdef DEBUG
        printf("strNum = %s\n", strNum);
    #endif // DEBUG
    memset(strInputScreen, 0, sizeof(strInputScreen));
    strcat(strInputScreen, "= ");
    strcat(strInputScreen, strNum);
    if(flagDivideZero)
        strcat(strInputScreen, "\nyou can't divide by zero / на ноль делить нельзя");
    SetWindowText(inputScreen, strInputScreen);
}

void ShowNumMasInputScreen()
{
    if (flagBtBack)
    {
        snprintf(strInputScreen, sizeof(strInputScreen), makeSpecificator(masNum[countMasNum]), masNum[countMasNum]);
    }
    else
    {
        char strSpecForInputScreen[10] = "%.";
        char strNumSpecForInputScreen[3] = {};
        sprintf(strNumSpecForInputScreen, "%d", (int)(countRazr - 1.0) );
        strcat(strSpecForInputScreen, strNumSpecForInputScreen);
        strcat(strSpecForInputScreen, "f");

        snprintf(strInputScreen, sizeof(strInputScreen), strSpecForInputScreen, masNum[countMasNum]);
    }

    SetWindowText(inputScreen, strInputScreen);
}
