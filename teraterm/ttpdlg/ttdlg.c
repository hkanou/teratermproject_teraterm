/*
 * Copyright (C) 1994-1998 T. Teranishi
 * (C) 2004- TeraTerm Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* IPv6 modification is Copyright(C) 2000 Jun-ya Kato <kato@win6.jp> */

/* TTDLG.DLL, dialog boxes */
#include "teraterm.h"
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <assert.h>

#include "tttypes.h"
#include "ttlib.h"
#include "dlglib.h"
#include "ttcommon.h"
#include "dlg_res.h"
#include "codeconv.h"
#include "helpid.h"
#include "asprintf.h"
#include "win32helper.h"
#include "compat_win.h"
#include "ttlib_charset.h"
#include "ttwinman.h"
#include "resize_helper.h"

#include "ttdlg.h"

#undef DialogBoxParam
#define DialogBoxParam(p1,p2,p3,p4,p5) \
	TTDialogBoxParam(p1,p2,p3,p4,p5)
#undef DialogBox
#define DialogBox(p1,p2,p3,p4) \
	TTDialogBox(p1,p2,p3,p4)
#undef EndDialog
#define EndDialog(p1,p2) \
	TTEndDialog(p1, p2)

static INT_PTR CALLBACK DirDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static const DlgTextInfo TextInfos[] = {
		{ 0, "DLG_DIR_TITLE" },
		{ IDC_DIRCURRENT_LABEL, "DLG_DIR_CURRENT" },
		{ IDC_DIRNEW_LABEL, "DLG_DIR_NEW" },
		{ IDOK, "BTN_OK" },
		{ IDCANCEL, "BTN_CANCEL" },
		{ IDC_DIRHELP, "BTN_HELP" },
	};

	switch (Message) {
		case WM_INITDIALOG: {
			PTTSet ts;
			wchar_t *CurDir;
			RECT R;
			HDC TmpDC;
			SIZE s;
			HWND HDir, HSel, HOk, HCancel, HHelp;
			POINT D, B, S;
			int WX, WY, WW, WH, CW, DW, DH, BW, BH, SW, SH;

			ts = (PTTSet)lParam;
			CurDir = ts->FileDirW;
			SetWindowLongPtr(Dialog, DWLP_USER, lParam);
			SetDlgTextsW(Dialog, TextInfos, _countof(TextInfos), ts->UILanguageFileW);
			SetDlgItemTextW(Dialog, IDC_DIRCURRENT, CurDir);

// adjust dialog size
			// get size of current dir text
			HDir = GetDlgItem(Dialog, IDC_DIRCURRENT);
			GetWindowRect(HDir,&R);
			D.x = R.left;
			D.y = R.top;
			ScreenToClient(Dialog,&D);
			DH = R.bottom-R.top;
			TmpDC = GetDC(Dialog);
			GetTextExtentPoint32W(TmpDC,CurDir,(int)wcslen(CurDir),&s);
			ReleaseDC(Dialog,TmpDC);
			DW = s.cx + s.cx/10;

			// get button size
			HOk = GetDlgItem(Dialog, IDOK);
			HCancel = GetDlgItem(Dialog, IDCANCEL);
			HHelp = GetDlgItem(Dialog, IDC_DIRHELP);
			GetWindowRect(HHelp,&R);
			B.x = R.left;
			B.y = R.top;
			ScreenToClient(Dialog,&B);
			BW = R.right-R.left;
			BH = R.bottom-R.top;

			// calc new dialog size
			GetWindowRect(Dialog,&R);
			WX = R.left;
			WY = R.top;
			WW = R.right-R.left;
			WH = R.bottom-R.top;
			GetClientRect(Dialog,&R);
			CW = R.right-R.left;
			if (D.x+DW < CW) {
				DW = CW-D.x;
			}
			WW = WW + D.x+DW - CW;

			// resize current dir text
			MoveWindow(HDir,D.x,D.y,DW,DH,TRUE);
			// move buttons
			MoveWindow(HOk,(D.x+DW-4*BW)/2,B.y,BW,BH,TRUE);
			MoveWindow(HCancel,(D.x+DW-BW)/2,B.y,BW,BH,TRUE);
			MoveWindow(HHelp,(D.x+DW+2*BW)/2,B.y,BW,BH,TRUE);
			// resize edit box
			HDir = GetDlgItem(Dialog, IDC_DIRNEW);
			GetWindowRect(HDir,&R);
			D.x = R.left;
			D.y = R.top;
			ScreenToClient(Dialog,&D);
			DW = R.right-R.left;
			if (DW<s.cx) {
				DW = s.cx;
			}
			MoveWindow(HDir,D.x,D.y,DW,R.bottom-R.top,TRUE);
			// select dir button
			HSel = GetDlgItem(Dialog, IDC_SELECT_DIR);
			GetWindowRect(HSel, &R);
			SW = R.right-R.left;
			SH = R.bottom-R.top;
			S.x = R.bottom;
			S.y = R.top;
			ScreenToClient(Dialog, &S);
			MoveWindow(HSel, D.x + DW + 8, S.y, SW, SH, TRUE);
			WW = WW + SW;

			// resize dialog
			MoveWindow(Dialog,WX,WY,WW,WH,TRUE);

			CenterWindow(Dialog, GetParent(Dialog));

			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					PTTSet ts = (PTTSet)GetWindowLongPtr(Dialog,DWLP_USER);
					BOOL OK = FALSE;
					wchar_t *current_dir;
					wchar_t *new_dir;
					hGetCurrentDirectoryW(&current_dir);
					hGetDlgItemTextW(Dialog, IDC_DIRNEW, &new_dir);
					if (wcslen(new_dir) > 0) {
						wchar_t *FileDirExpanded;
						hExpandEnvironmentStringsW(new_dir, &FileDirExpanded);
						if (DoesFolderExistW(FileDirExpanded)) {
							free(ts->FileDirW);
							ts->FileDirW = new_dir;
							OK = TRUE;
						}
						else {
							free(new_dir);
						}
						free(FileDirExpanded);
					}
					SetCurrentDirectoryW(current_dir);
					free(current_dir);
					if (OK) {
						EndDialog(Dialog, 1);
					}
					else {
						static const TTMessageBoxInfoW info = {
							"Tera Term",
							"MSG_TT_ERROR", L"Tera Term: Error",
							"MSG_FIND_DIR_ERROR", L"Cannot find directory",
							MB_ICONEXCLAMATION
						};
						TTMessageBoxW(Dialog, &info, ts->UILanguageFileW);
					}
					return TRUE;
				}

				case IDCANCEL:
					EndDialog(Dialog, 0);
					return TRUE;

				case IDC_SELECT_DIR: {
					PTTSet ts = (PTTSet)GetWindowLongPtr(Dialog,DWLP_USER);
					wchar_t *uimsgW;
					wchar_t *buf;
					wchar_t *FileDirExpanded;
					GetI18nStrWW("Tera Term", "DLG_SELECT_DIR_TITLE", L"Select new directory", ts->UILanguageFileW, &uimsgW);
					hGetDlgItemTextW(Dialog, IDC_DIRNEW, &buf);
					if (buf[0] == 0) {
						free(buf);
						hGetDlgItemTextW(Dialog, IDC_DIRCURRENT, &buf);
					}
					hExpandEnvironmentStringsW(buf, &FileDirExpanded);
					free(buf);
					if (doSelectFolderW(Dialog, FileDirExpanded, uimsgW, &buf)) {
						SetDlgItemTextW(Dialog, IDC_DIRNEW, buf);
						free(buf);
					}
					free(FileDirExpanded);
					free(uimsgW);
					return TRUE;
				}

				case IDC_DIRHELP:
					PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,HlpFileChangeDir,0);
					break;
			}
	}
	return FALSE;
}

static const wchar_t **LangUIList = NULL;
#define LANG_EXT L".lng"

static const wchar_t *get_lang_folder()
{
	return (IsWindowsNTKernel()) ? L"lang_utf16le" : L"lang";
}

// メモリフリー
static void free_lang_ui_list()
{
	if (LangUIList) {
		const wchar_t **p = LangUIList;
		while (*p) {
			free((void *)*p);
			p++;
		}
		free((void *)LangUIList);
		LangUIList = NULL;
	}
}

static int make_sel_lang_ui(const wchar_t *dir)
{
	int    i;
	int    file_num;
	wchar_t *fullpath;
	HANDLE hFind;
	WIN32_FIND_DATAW fd;

	free_lang_ui_list();

	aswprintf(&fullpath, L"%s\\%s\\*%s", dir, get_lang_folder(), LANG_EXT);

	file_num = 0;
	hFind = FindFirstFileW(fullpath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				file_num++;
			}
		} while(FindNextFileW(hFind, &fd));
		FindClose(hFind);
	}

	file_num++;  // NULL
	LangUIList = calloc(file_num, sizeof(wchar_t *));

	i = 0;
	hFind = FindFirstFileW(fullpath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				LangUIList[i++] = _wcsdup(fd.cFileName);
			}
		} while(FindNextFileW(hFind, &fd) && i < file_num);
		FindClose(hFind);
	}
	LangUIList[i] = NULL;
	free(fullpath);

	return i;
}

static int get_sel_lang_ui(const wchar_t **list, const wchar_t *selstr)
{
	size_t n = 0;
	const wchar_t **p = list;

	if (selstr == NULL || selstr[0] == '\0') {
		n = 0;  // English
	} else {
		while (*p) {
			if (wcsstr(selstr, *p)) {
				n = p - list;
				break;
			}
			p++;
		}
	}

	return (int)(n + 1);  // 1origin
}

static INT_PTR CALLBACK GenDlg(HWND Dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
	static const DlgTextInfo TextInfos[] = {
		{ 0, "DLG_GEN_TITLE" },
		{ IDC_GENPORT_LABEL, "DLG_GEN_PORT" },
		{ IDC_GENLANGLABEL, "DLG_GEN_LANG" },
		{ IDC_GENLANGUI_LABEL, "DLG_GEN_LANG_UI" },
		{ IDOK, "BTN_OK" },
		{ IDCANCEL, "BTN_CANCEL" },
		{ IDC_GENHELP, "BTN_HELP" },
	};
	static int langui_sel = 1, uilist_count = 0;
	PTTSet ts;
	WORD w;

	switch (Message) {
		case WM_INITDIALOG:
			ts = (PTTSet)lParam;
			SetWindowLongPtr(Dialog, DWLP_USER, lParam);

			SetDlgTextsW(Dialog, TextInfos, _countof(TextInfos), ts->UILanguageFileW);

			SendDlgItemMessageA(Dialog, IDC_GENPORT, CB_ADDSTRING,
			                   0, (LPARAM)"TCP/IP");
			for (w=1;w<=ts->MaxComPort;w++) {
				char Temp[8];
				_snprintf_s(Temp, sizeof(Temp), _TRUNCATE, "COM%d", w);
				SendDlgItemMessageA(Dialog, IDC_GENPORT, CB_ADDSTRING,
				                   0, (LPARAM)Temp);
			}
			if (ts->PortType==IdSerial) {
				if (ts->ComPort <= ts->MaxComPort) {
					w = ts->ComPort;
				}
				else {
					w = 1; // COM1
				}
			}
			else {
				w = 0; // TCP/IP
			}
			SendDlgItemMessage(Dialog, IDC_GENPORT, CB_SETCURSEL,w,0);

			if ((ts->MenuFlag & MF_NOLANGUAGE)==0) {
				int sel = 0;
				int i;
				ShowDlgItem(Dialog,IDC_GENLANGLABEL,IDC_GENLANG);
				for (i = 0;; i++) {
					const TLanguageList *lang = GetLanguageList(i);
					if (lang == NULL) {
						break;
					}
					if (ts->Language == lang->language) {
						sel = i;
					}
					SendDlgItemMessageA(Dialog, IDC_GENLANG, CB_ADDSTRING, 0, (LPARAM)lang->str);
					SendDlgItemMessageA(Dialog, IDC_GENLANG, CB_SETITEMDATA, i, (LPARAM)lang->language);
				}
				SendDlgItemMessage(Dialog, IDC_GENLANG, CB_SETCURSEL, sel, 0);
			}

			// 最初に指定されている言語ファイルの番号を覚えておく。
			uilist_count = make_sel_lang_ui(ts->ExeDirW);
			langui_sel = get_sel_lang_ui(LangUIList, ts->UILanguageFileW_ini);
			if (LangUIList[0] != NULL) {
				int i = 0;
				while (LangUIList[i] != 0) {
					SendDlgItemMessageW(Dialog, IDC_GENLANG_UI, CB_ADDSTRING, 0, (LPARAM)LangUIList[i]);
					i++;
				}
				SendDlgItemMessage(Dialog, IDC_GENLANG_UI, CB_SETCURSEL, langui_sel - 1, 0);
			}
			else {
				EnableWindow(GetDlgItem(Dialog, IDC_GENLANG_UI), FALSE);
			}

			CenterWindow(Dialog, GetParent(Dialog));

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					ts = (PTTSet)GetWindowLongPtr(Dialog,DWLP_USER);
					if (ts!=NULL) {
						w = (WORD)GetCurSel(Dialog, IDC_GENPORT);
						if (w>1) {
							ts->PortType = IdSerial;
							ts->ComPort = w-1;
						}
						else {
							ts->PortType = IdTCPIP;
						}

						if ((ts->MenuFlag & MF_NOLANGUAGE)==0) {
							WORD language;
							w = (WORD)GetCurSel(Dialog, IDC_GENLANG);
							language = (int)SendDlgItemMessageA(Dialog, IDC_GENLANG, CB_GETITEMDATA, w - 1, 0);

							// Language が変更されたとき、
							// KanjiCode/KanjiCodeSend を変更先の Language に存在する値に置き換える
							if (1 <= language && language < IdLangMax && language != ts->Language) {
								WORD KanjiCode = ts->KanjiCode;
								WORD KanjiCodeSend = ts->KanjiCodeSend;
								ts->KanjiCode = KanjiCodeTranslate(language,KanjiCode);
								ts->KanjiCodeSend = KanjiCodeTranslate(language,KanjiCodeSend);

								ts->Language = language;
							}

						}

						// 言語ファイルが変更されていた場合
						w = (WORD)GetCurSel(Dialog, IDC_GENLANG_UI);
						if (1 <= w && w <= uilist_count && w != langui_sel) {
							aswprintf(&ts->UILanguageFileW_ini, L"%s\\%s", get_lang_folder(), LangUIList[w - 1]);
							WideCharToACP_t(ts->UILanguageFileW_ini, ts->UILanguageFile_ini, sizeof(ts->UILanguageFile_ini));

							ts->UILanguageFileW = GetUILanguageFileFullW(ts->ExeDirW, ts->UILanguageFileW_ini);
							WideCharToACP_t(ts->UILanguageFileW, ts->UILanguageFile, sizeof(ts->UILanguageFile));

							// タイトルの更新を行う。(2014.2.23 yutaka)
							PostMessage(GetParent(Dialog),WM_USER_CHANGETITLE,0,0);
						}
					}

					// TTXKanjiMenu は Language を見てメニューを表示するので、変更の可能性がある
					// OK 押下時にメニュー再描画のメッセージを飛ばすようにした。 (2007.7.14 maya)
					// 言語ファイルの変更時にメニューの再描画が必要 (2012.5.5 maya)
					PostMessage(GetParent(Dialog),WM_USER_CHANGEMENU,0,0);

					EndDialog(Dialog, 1);
					return TRUE;

				case IDCANCEL:
					EndDialog(Dialog, 0);
					return TRUE;

				case IDC_GENHELP:
					PostMessage(GetParent(Dialog),WM_USER_DLGHELP2,HlpSetupGeneral,0);
					break;
			}
			break;

		case WM_DESTROY:
			free_lang_ui_list();
			break;
	}
	return FALSE;
}

BOOL WINAPI _ChangeDirectory(HWND WndParent, PTTSet ts)
{
	return
		(BOOL)DialogBoxParam(hInst,
		                     MAKEINTRESOURCE(IDD_DIRDLG),
		                     WndParent, DirDlg, (LPARAM)ts);
}

BOOL WINAPI _SetupGeneral(HWND WndParent, PTTSet ts)
{
	return
		(BOOL)DialogBoxParam(hInst,
		                     MAKEINTRESOURCE(IDD_GENDLG),
		                     WndParent, GenDlg, (LPARAM)ts);
}
