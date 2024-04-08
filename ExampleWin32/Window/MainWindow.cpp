#include "MainWindow.h"

#include <opencv2/opencv.hpp>

#include "WndEnumeration.h"
#include "macro.h"

#include "windowsx.h"

namespace {


LRESULT CALLBACK WndProc(
	HWND   hwnd,
	UINT   msg,
	WPARAM wParam,
	LPARAM lParam) {

	return 0;
}


} // namespace

namespace ohms {

MainWindow::MainWindow() {
	g_app = ohms::wgc::ICapture::getInstance();
}

MainWindow::~MainWindow() {
	stopCapture();
	g_app = nullptr;
}

bool ohms::MainWindow::create(int nShowCmd) noexcept {
	if (!Parent::create())
		return false;
	SetWindowTextW(m_hwnd, L"CaptureWithOpenCV");

	//SetTimer(hMain, 1, 300, NULL);
	SetTimer(m_hwnd, 1, 60, NULL);

	return true;
}

void MainWindow::destroy() noexcept {
	return Parent::destroy();
}

LRESULT MainWindow::procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_CREATE:
	{
		hFont = CreateFontW(0, 0, 0, 0, FW_DONTCARE,
							FALSE, FALSE, FALSE,
							GB2312_CHARSET,
							OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS,
							CLEARTYPE_QUALITY,
							DEFAULT_PITCH | FF_DONTCARE,
							L"Segoe UI");

		// Create combo box
		hComboBoxHwnd = CreateWindowW(
			WC_COMBOBOXW, L"",
			CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			150, 10, 600, 300,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hComboBoxHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonRf = CreateWindowW(
			WC_BUTTONW, L"Refresh",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 10, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonRf, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonStop = CreateWindowW(
			WC_BUTTONW, L"Stop",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 70, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonStop, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonSample = CreateWindowW(
			WC_BUTTONW, L"Decimation",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 130, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSample, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSample, FALSE);

		hButtonSwitchClient = CreateWindowW(
			WC_BUTTONW, L"Switch Client",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 190, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSwitchClient, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSwitchClient, FALSE);

		hButtonBigger = CreateWindowW(
			WC_BUTTONW, L"Scale +",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 250, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonBigger, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonBigger, FALSE);

		hButtonSmaller = CreateWindowW(
			WC_BUTTONW, L"Scale -",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 310, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSmaller, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSmaller, FALSE);

		hButtonSaveC3 = CreateWindowW(
			WC_BUTTONW, L"Save BGR",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 370, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSaveC3, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSaveC3, FALSE);

		hButtonSave = CreateWindowW(
			WC_BUTTONW, L"Save BGRA",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 430, 100, 40,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSave, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSave, FALSE);

		SendMessageW(hwnd, WM_OHMS_REFRESH_COMBOX, 0, 0);
		break;
	}

	case WM_DESTROY:
	{
		DestroyWindow(hButtonSwitchClient);
		DestroyWindow(hButtonSaveC3);
		DestroyWindow(hButtonSave);
		DestroyWindow(hButtonSmaller);
		DestroyWindow(hButtonBigger);
		DestroyWindow(hButtonSample);
		DestroyWindow(hButtonStop);
		DestroyWindow(hButtonRf);
		DestroyWindow(hComboBoxHwnd);
		DeleteObject(hFont);
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkMode(hdc, TRANSPARENT);

		HGDIOBJ hOldFont = SelectObject(hdc, hFont);

		WCHAR str[1024];
		RECT rect;

		swprintf_s(str, 1024, (nowPlay != -1) ? L"Running" : L"Stopped");
		rect = { 120, 80, 220, 120 };
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
		//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		swprintf_s(str, 1024, isSample ? L"Now:\nDecimation" : L"Now:\nNormal");
		rect = { 120, 130, 220, 180 };
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
		//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		swprintf_s(str, 1024, isClient ? L"Now:\nClient" : L"Now:\nWindow");
		rect = { 120, 190, 220, 280 };
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
		//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		swprintf_s(str, 1024, L"Scale: %d %%", scale * (isSample ? 10 : 1));
		rect = { 120, 290, 220, 500 };
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOCLIP | DT_CALCRECT);
		//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		rect = { 230, 80, 750, 500 };
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		if (nowPlay != -1) {
			swprintf_s(str, 1024, L"Class:\n%s\n\nTitle:\n%s",
					   m_windows[nowPlay].getClassName().c_str(),
					   m_windows[nowPlay].getTitle().c_str());
		}
		else {
			swprintf_s(str, 1024, L"Class:\n\n\nTitle:\n");
		}
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS);

		SelectObject(hdc, hOldFont);
		EndPaint(hwnd, &ps);

		break;
	}

	case WM_CLOSE:
		SendMessageW(hwnd, WM_OHMS_STOP_CAPTURE, 0, 0);
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
		{
			const LRESULT index = SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);

			if (nowPlay == index)
				break;

			stopCapture(true);

			HWND dst = m_windows.at(index).getHWnd();

			if (IsWindow(dst)) {
				if (!IsIconic(dst)) {
					if (g_app->startCapture(dst)) {
						g_app->askForRefresh();
						//ohms::global::g_app->setDecimationMode(isSample);
						//ohms::global::g_app->setShowScale(scale);

						nowPlay = index;
					}
					else {
						stopCapture();
						MessageBoxW(hwnd, L"This window cannot be captured.", L"ERROR", MB_ICONERROR);
					}
				}
				else {
					stopCapture();
					MessageBoxW(hwnd, L"Please restore minimized window before capture.", L"WARNING", MB_ICONWARNING);
				}
			}
			else {
				stopCapture();
				SendMessageW(hwnd, WM_OHMS_REFRESH_COMBOX, 0, 0);
			}

			InvalidateRect(hwnd, NULL, true);

			break;
		}
		case BN_CLICKED:
		{

			if ((HWND)lParam == hButtonRf) {
				SendMessageW(hwnd, WM_OHMS_STOP_CAPTURE, 0, 0);
				SendMessageW(hwnd, WM_OHMS_REFRESH_COMBOX, 0, 0);
			}
			else if ((HWND)lParam == hButtonStop) {
				SendMessageW(hwnd, WM_OHMS_STOP_CAPTURE, 0, 0);
			}
			else if ((HWND)lParam == hButtonSample) {
				isSample = !isSample;
				//ohms::global::g_app->setDecimationMode(isSample);
				SetWindowTextW(hButtonSample, isSample ? L"Normal" : L"Decimation");
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonBigger) {
				if (scale < 200)
					scale += 10;
				//ohms::global::g_app->setShowScale(scale);
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonSmaller) {
				if (scale > 10)
					scale -= 10;
				//ohms::global::g_app->setShowScale(scale);
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonSave) {
				if (nowPlay == -1) {
					MessageBoxW(hwnd, L"Please choose a window before save image.", L"ERROR", MB_ICONERROR);
				}
				else {
					//if (!ohms::global::g_app->saveNow(false, saveCount++)) {
					//	MessageBoxW(hwnd, L"Failed saving.", L"ERROR", MB_ICONERROR);
					//}
				}
			}
			else if ((HWND)lParam == hButtonSaveC3) {
				if (nowPlay == -1) {
					MessageBoxW(hwnd, L"Please choose a window before save image.", L"ERROR", MB_ICONERROR);
				}
				else {
					//if (!ohms::global::g_app->saveNow(true, saveCount++)) {
					//	MessageBoxW(hwnd, L"Failed saving.", L"ERROR", MB_ICONERROR);
					//}
				}
			}
			else if ((HWND)lParam == hButtonSwitchClient) {
				isClient = !isClient;
				g_app->setClipToClientArea(isClient);
				InvalidateRect(hwnd, NULL, true);
			}
			break;
		}
		default:
			break;
		}
		break;

	case WM_TIMER:
		if (g_app->isRefreshed()) { // refresh过再处理画面才有意义
			g_app->askForRefresh();

			cv::Mat m_mat;
			if (g_app->copyMatTo(m_mat, false)) { // 不要求转换为BGR
				cv::imshow("test", m_mat);
			}

		}
		break;

	case WM_OHMS_REFRESH_COMBOX:
		refreshCombox();
		break;

	case WM_OHMS_STOP_CAPTURE:
		stopCapture();
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}
	return 0;
}

void MainWindow::refreshCombox() {
	m_windows.clear();
	ohms::EnumerateWindows(m_windows);

	SendMessageW(hComboBoxHwnd, CB_RESETCONTENT, 0, 0);

	for (std::vector<ohms::WindowInfo>::iterator i = m_windows.begin(), n = m_windows.end(); i != n; ++i) {
		if (i->getHWnd() == m_hwnd) {
			m_windows.erase(i);
			break;
		}
	}

	// Populate combo box
	for (const ohms::WindowInfo& window : m_windows) {
		SendMessageW(hComboBoxHwnd, CB_ADDSTRING, 0, (LPARAM)window.getTitle().c_str());
	}
}

void MainWindow::stopCapture(bool special) {
	nowPlay = -1;
	g_app->stopCapture();

	if (!special)
		SendMessageW(hComboBoxHwnd, CB_SETCURSEL, -1, 0);

	cv::destroyAllWindows();

	if (!special)
		InvalidateRect(m_hwnd, NULL, true);
}

} // namespace ohms
