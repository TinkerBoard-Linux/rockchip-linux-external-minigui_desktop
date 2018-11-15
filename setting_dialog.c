/*
 * This is a every simple sample for MiniGUI.
 * It will create a main window and display a string of "Hello, world!" in it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h> 
#include <math.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "common.h"

static BITMAP list_sel_bmap;
static int list_sel = 0;
static const char *pTitle = "设置";
static int batt = 0;
static const char *name_list[] = {
    "语言设置",
    "音效设置",
    "关屏设置",
    "背光设置",
    "恢复默认设置",
    "系统信息"
};

static int loadres(void)
{
    int i;
    char img[128];
    char respath[] = UI_IMAGE_PATH;

    snprintf(img, sizeof(img), "%slist_sel.png", respath);
    //printf("%s\n", img);
    if (LoadBitmap(HDC_SCREEN, &list_sel_bmap, img))
        return -1;

    return 0;
}

static void unloadres(void)
{
    UnloadBitmap(&list_sel_bmap);
}

static LRESULT setting_dialog_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;

    //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
    switch (message) {
    case MSG_INITDIALOG: {
    	  DWORD bkcolor;
        HWND hFocus = GetDlgDefPushButton(hWnd);
        loadres();
        bkcolor = GetWindowElementPixel(hWnd, WE_BGC_WINDOW);
        SetWindowBkColor(hWnd, bkcolor);
        if (hFocus)
            SetFocus(hFocus);

        SetTimer(hWnd, _ID_TIMER_SETTING, 100);
        return 0;
    }
    case MSG_TIMER: {
        if (wParam == _ID_TIMER_SETTING) {
            if (batt != battery) {
                batt = battery;
                InvalidateRect(hWnd, &msg_rcBatt, TRUE);
            }
        }
        break;
    }
    case MSG_PAINT:
    {
        int i;
        int page;
        struct file_node *file_node_temp;
        gal_pixel old_brush;
        gal_pixel pixle = 0xffffffff;

        hdc = BeginPaint(hWnd);
        old_brush = SetBrushColor(hdc, pixle);
        FillBoxWithBitmap(hdc, BG_PINT_X,
                               BG_PINT_Y, BG_PINT_W,
                               BG_PINT_H, &background_bmap);
        FillBoxWithBitmap(hdc, BATT_PINT_X, BATT_PINT_Y,
                               BATT_PINT_W, BATT_PINT_H,
                               &batt_bmap[batt]);

        SetBkColor(hdc, COLOR_transparent);
        SetBkMode(hdc,BM_TRANSPARENT);
        SetTextColor(hdc, RGB2Pixel(hdc, 0xff, 0xff, 0xff));
        DrawText(hdc, pTitle, -1, &msg_rcTitle, DT_TOP);
        FillBox(hdc, 0, 46, LCD_W, 2);

        for (i = 0; i < sizeof(name_list) / sizeof(char *); i++) {
            RECT msg_rcFilename;

            msg_rcFilename.left = SETTING_LIST_STR_PINT_X;
            msg_rcFilename.top = SETTING_LIST_STR_PINT_Y + SETTING_LIST_STR_PINT_SPAC * i;
            msg_rcFilename.right = LCD_W - msg_rcFilename.left;
            msg_rcFilename.bottom = msg_rcFilename.top + SETTING_LIST_STR_PINT_H;

            if (i == list_sel)
                FillBoxWithBitmap(hdc, 0, msg_rcFilename.top - 9, LCD_W, SETTING_LIST_SEL_PINT_H, &list_sel_bmap);
            DrawText(hdc, name_list[i], -1, &msg_rcFilename, DT_TOP);
        }

        SetBrushColor(hdc, old_brush);
        EndPaint(hWnd, hdc);
        break;
    }
    case MSG_KEYDOWN:
        //printf("%s message = 0x%x, 0x%x, 0x%x\n", __func__, message, wParam, lParam);
        switch (wParam) {
            case SCANCODE_MODE:
            case SCANCODE_B:
                EndDialog(hWnd, wParam);
                break;
            case SCANCODE_MUTE:
                break;
            case SCANCODE_VOLUP:
            case SCANCODE_CURSORBLOCKDOWN:
                if (list_sel < (sizeof(name_list) / sizeof(char *) - 1))
                    list_sel++;
                else
                    list_sel = 0;
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
                break;
            case SCANCODE_VOLDOWN:
            case SCANCODE_CURSORBLOCKUP:
                 if (list_sel > 0)
                    list_sel--;
                else
                    list_sel = sizeof(name_list) / sizeof(char *) - 1;
                InvalidateRect(hWnd, &msg_rcBg, TRUE);
                break;
            case SCANCODE_PLAY:
            case SCANCODE_A:
                switch (list_sel) {
                    case 0:
                        creat_setting_language_dialog(hWnd);
                        break;
                    case 1:
                        creat_setting_eq_dialog(hWnd);
                        break;
                    case 2:
                        creat_setting_screenoff_dialog(hWnd);
                        break;
                    case 3:
                        creat_setting_backlight_dialog(hWnd);
                        break;
                    case 4:
                        break;
                    case 5:
                        creat_setting_version_dialog(hWnd);
                        break;
                }
                break;
        }
        break;
    case MSG_COMMAND: {
        break;
    }
    case MSG_DESTROY:
        unloadres();
        break;
    }

    return DefaultDialogProc(hWnd, message, wParam, lParam);
}

void creat_setting_dialog(HWND hWnd)
{
    DLGTEMPLATE DesktopDlg = {WS_VISIBLE, WS_EX_NONE | WS_EX_AUTOSECONDARYDC,
    	                        0, 0,
    	                        LCD_W, LCD_H,
                              DESKTOP_DLG_STRING, 0, 0, 0, NULL, 0};
    //DesktopDlg.controls = DesktopCtrl;

    DialogBoxIndirectParam(&DesktopDlg, hWnd, setting_dialog_proc, 0L);
}
