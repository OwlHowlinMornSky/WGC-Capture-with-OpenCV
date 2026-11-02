/*
*    WGC-Capture-with-OpenCV
*
*     Copyright 2023-2024  Tyler Parret True
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*
* @Authors
*    Tyler Parret True <mysteryworldgod@outlook.com><https://github.com/OwlHowlinMornSky>
*/
#include "MainWindow.h"

#include <opencv2/opencv.hpp>

#include <commdlg.h>

#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "WndEnumeration.h"
#include "macro.h"

#include "windowsx.h"

namespace {

constexpr int kDecimationFactor = 10;

} // namespace

namespace ohms {

MainWindow::MainWindow() {
	r_factory = wgc::IFactory::createInstanceNoThrow(false);
	if (r_factory) {
		try {
			r_capture = r_factory->createCapturer().lock();
		}
		catch (...) {
			r_capture.reset();
		}
	}
}

MainWindow::~MainWindow() {
	stopCapture();
	if (r_factory && r_capture)
		r_factory->destroyCapturer(r_capture);
	r_capture.reset();
	r_factory.reset();
}

bool ohms::MainWindow::create(int nShowCmd) noexcept {
	if (!r_factory || !r_capture)
		return false;
	if (!Parent::create())
		return false;

	SetWindowTextW(m_hwnd, L"CaptureWithOpenCV");

	if (SetTimer(m_hwnd, 1, 60, NULL) == 0) {
		destroy();
		return false;
	}

	return true;
}

void MainWindow::destroy() noexcept {
	return Parent::destroy();
}

LRESULT MainWindow::procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_CREATE:
	{
		m_hwnd = hwnd;
		hFont = CreateFontW(0, 0, 0, 0, FW_DONTCARE,
			FALSE, FALSE, FALSE,
			GB2312_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Segoe UI");

		// Window source combo.
		hComboBoxHwnd = CreateWindowW(
			WC_COMBOBOXW, L"",
			CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			150, 10, 440, 200,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hComboBoxHwnd, WM_SETFONT, (WPARAM)hFont, TRUE);

		// Monitor source combo.
		hMonitorCombo = CreateWindowW(
			WC_COMBOBOXW, L"",
			CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			150, 50, 440, 200,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hMonitorCombo, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonScreen = CreateWindowW(
			WC_BUTTONW, L"Capture Screen",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			600, 50, 140, 30,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonScreen, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonRf = CreateWindowW(
			WC_BUTTONW, L"Refresh",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 10, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonRf, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonStop = CreateWindowW(
			WC_BUTTONW, L"Stop",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 55, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonStop, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonSample = CreateWindowW(
			WC_BUTTONW, L"Decimation",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 100, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSample, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonSwitchClient = CreateWindowW(
			WC_BUTTONW, L"Switch Client",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 145, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSwitchClient, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonBigger = CreateWindowW(
			WC_BUTTONW, L"Scale +",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 190, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonBigger, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonSmaller = CreateWindowW(
			WC_BUTTONW, L"Scale -",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 235, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSmaller, WM_SETFONT, (WPARAM)hFont, TRUE);

		hButtonSaveC3 = CreateWindowW(
			WC_BUTTONW, L"Save BGR",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 280, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSaveC3, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSaveC3, FALSE);

		hButtonSave = CreateWindowW(
			WC_BUTTONW, L"Save BGRA",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			10, 325, 100, 35,
			hwnd, NULL, GetModuleHandleW(NULL), NULL);
		SendMessageW(hButtonSave, WM_SETFONT, (WPARAM)hFont, TRUE);
		Button_Enable(hButtonSave, FALSE);

		refreshCombox();
		updateControls();
		break;
	}

	case WM_DESTROY:
	{
		DestroyWindow(hButtonScreen);
		DestroyWindow(hButtonSwitchClient);
		DestroyWindow(hButtonSaveC3);
		DestroyWindow(hButtonSave);
		DestroyWindow(hButtonSmaller);
		DestroyWindow(hButtonBigger);
		DestroyWindow(hButtonSample);
		DestroyWindow(hButtonStop);
		DestroyWindow(hButtonRf);
		DestroyWindow(hMonitorCombo);
		DestroyWindow(hComboBoxHwnd);
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		SetBkMode(hdc, TRANSPARENT);

		HGDIOBJ hOldFont = SelectObject(hdc, hFont);

		WCHAR str[2048];
		RECT rect;

		const wchar_t* state =
			nowSource == SourceKind::None ? L"Stopped" : L"Running";
		const wchar_t* source =
			nowSource == SourceKind::Window ? L"Window" :
			nowSource == SourceKind::Monitor ? L"Screen" : L"None";

		swprintf_s(
			str,
			L"State: %s\nSource: %s\nDecimation: %s\nScale: %d %%\nClient area: %s",
			state,
			source,
			isSample ? L"10x" : L"Off",
			scale,
			nowSource == SourceKind::Monitor
				? L"N/A (screen)"
				: (isClient ? L"On" : L"Off")
		);
		rect = { 10, 380, 220, 510 };
		DrawTextW(hdc, str, -1, &rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

		if (!m_statusMessage.empty()) {
			rect = { 10, 505, 780, 555 };
			DrawTextW(
				hdc, m_statusMessage.c_str(), -1, &rect,
				DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_WORD_ELLIPSIS
			);
		}

		rect = { 230, 90, 750, 500 };
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

		rect.left += 8;
		rect.top += 8;
		rect.right -= 8;
		rect.bottom -= 8;

		if (nowSource == SourceKind::Window &&
			nowPlay >= 0 &&
			static_cast<size_t>(nowPlay) < m_windows.size()) {
			const auto& window = m_windows[static_cast<size_t>(nowPlay)];
			swprintf_s(
				str,
				L"Window\n\nClass:\n%s\n\nTitle:\n%s",
				window.getClassName().c_str(),
				window.getTitle().c_str()
			);
		}
		else if (nowSource == SourceKind::Monitor &&
			nowMonitor >= 0 &&
			static_cast<size_t>(nowMonitor) < m_monitors.size()) {
			const auto& monitor = m_monitors[static_cast<size_t>(nowMonitor)];
			swprintf_s(
				str,
				L"Screen\n\nDevice:\n%s\n\nResolution:\n%d x %d\n\nPrimary:\n%s",
				monitor.getDeviceName().c_str(),
				monitor.getWidth(),
				monitor.getHeight(),
				monitor.isPrimary() ? L"Yes" : L"No"
			);
		}
		else {
			swprintf_s(
				str,
				L"No source selected.\n\n"
				L"Choose a window from the top list, or choose a monitor "
				L"and click \"Capture Screen\"."
			);
		}
		DrawTextW(
			hdc, str, -1, &rect,
			DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS
		);

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
			if ((HWND)lParam == hComboBoxHwnd) {
				const LRESULT index = SendMessageW(hComboBoxHwnd, CB_GETCURSEL, 0, 0);
				if (index < 0 || static_cast<size_t>(index) >= m_windows.size())
					break;
				if (nowSource == SourceKind::Window && nowPlay == index)
					break;
				startWindowCapture(static_cast<size_t>(index));
			}
			else if ((HWND)lParam == hMonitorCombo) {
				const LRESULT index = SendMessageW(hMonitorCombo, CB_GETCURSEL, 0, 0);
				if (index >= 0 && static_cast<size_t>(index) < m_monitors.size()) {
					m_statusMessage = L"Selected monitor. Click \"Capture Screen\" to start.";
					InvalidateRect(hwnd, NULL, true);
				}
			}
			break;
		}
		case BN_CLICKED:
		{
			if ((HWND)lParam == hButtonRf) {
				stopCapture();
				refreshCombox();
			}
			else if ((HWND)lParam == hButtonStop) {
				stopCapture();
			}
			else if ((HWND)lParam == hButtonSample) {
				isSample = !isSample;
				SetWindowTextW(hButtonSample, isSample ? L"Normal" : L"Decimation");
				if (!m_lastFrame.empty())
					showPreview();
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonBigger) {
				if (scale < 200)
					scale += 10;
				if (!m_lastFrame.empty())
					showPreview();
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonSmaller) {
				if (scale > 10)
					scale -= 10;
				if (!m_lastFrame.empty())
					showPreview();
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonSaveC3) {
				saveFrame(true);
			}
			else if ((HWND)lParam == hButtonSave) {
				saveFrame(false);
			}
			else if ((HWND)lParam == hButtonSwitchClient) {
				isClient = !isClient;
				if (r_capture && nowSource == SourceKind::Window)
					r_capture->setClipToClientArea(isClient);
				InvalidateRect(hwnd, NULL, true);
			}
			else if ((HWND)lParam == hButtonScreen) {
				const LRESULT index = SendMessageW(hMonitorCombo, CB_GETCURSEL, 0, 0);
				if (index < 0 || static_cast<size_t>(index) >= m_monitors.size()) {
					MessageBoxW(hwnd, L"Please select a monitor first.", L"WARNING", MB_ICONWARNING);
					break;
				}
				startMonitorCapture(static_cast<size_t>(index));
			}
			break;
		}
		default:
			break;
		}
		break;

	case WM_TIMER:
		if (r_capture && nowSource != SourceKind::None && !r_capture->isCapturing()) {
			// The target window closed, or the library stopped the session.
			nowSource = SourceKind::None;
			nowPlay = -1;
			nowMonitor = -1;
			cv::destroyAllWindows();
			m_statusMessage = L"Capture stopped (target closed).";
			updateControls();
			InvalidateRect(hwnd, NULL, true);
			break;
		}

		if (r_capture && r_capture->isRefreshed()) {
			try {
				cv::Mat frame;
				r_capture->copyMatTo(frame, false);
				r_capture->askForRefresh();
				if (!frame.empty()) {
					m_lastFrame = frame;
					updateControls();
					showPreview();
					InvalidateRect(hwnd, NULL, true);
				}
			}
			catch (...) {
				m_statusMessage = L"Failed to copy the captured frame.";
				InvalidateRect(hwnd, NULL, true);
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
	}
	return 0;
}

void MainWindow::refreshCombox() {
	try {
		m_windows.clear();
		m_monitors.clear();

		ohms::EnumerateWindows(m_windows);
		ohms::EnumerateMonitors(m_monitors);

		for (auto i = m_windows.begin(); i != m_windows.end(); ++i) {
			if (i->getHWnd() == m_hwnd) {
				m_windows.erase(i);
				break;
			}
		}
	}
	catch (...) {
		m_statusMessage = L"Failed to enumerate capture sources.";
	}

	SendMessageW(hComboBoxHwnd, CB_RESETCONTENT, 0, 0);
	for (const auto& window : m_windows) {
		SendMessageW(
			hComboBoxHwnd, CB_ADDSTRING, 0,
			(LPARAM)window.getTitle().c_str()
		);
	}

	SendMessageW(hMonitorCombo, CB_RESETCONTENT, 0, 0);
	for (size_t i = 0; i < m_monitors.size(); ++i) {
		SendMessageW(
			hMonitorCombo, CB_ADDSTRING, 0,
			(LPARAM)makeMonitorLabel(i).c_str()
		);
	}

	if (!m_monitors.empty()) {
		size_t selected = 0;
		for (size_t i = 0; i < m_monitors.size(); ++i) {
			if (m_monitors[i].isPrimary()) {
				selected = i;
				break;
			}
		}
		SendMessageW(hMonitorCombo, CB_SETCURSEL, selected, 0);
	}

	updateControls();
	InvalidateRect(m_hwnd, NULL, true);
}

void MainWindow::startWindowCapture(size_t index) {
	if (index >= m_windows.size())
		return;

	stopCapture(true);

	const HWND target = m_windows[index].getHWnd();
	if (!IsWindow(target)) {
		stopCapture();
		SendMessageW(m_hwnd, WM_OHMS_REFRESH_COMBOX, 0, 0);
		return;
	}
	if (IsIconic(target)) {
		stopCapture();
		MessageBoxW(
			m_hwnd,
			L"Please restore the minimized window before capture.",
			L"WARNING", MB_ICONWARNING
		);
		return;
	}

	try {
		r_capture->setClipToClientArea(isClient);
		if (!r_capture->startCaptureWindow(target)) {
			reportCaptureError(L"startCaptureWindow");
			stopCapture();
			return;
		}

		r_capture->askForRefresh();
		nowSource = SourceKind::Window;
		nowPlay = static_cast<LONG_PTR>(index);
		nowMonitor = -1;
		m_lastFrame.release();
		m_statusMessage = L"Window capture started.";
		updateControls();
		InvalidateRect(m_hwnd, NULL, true);
	}
	catch (...) {
		reportCaptureError(L"startCaptureWindow");
		stopCapture();
	}
}

void MainWindow::startMonitorCapture(size_t index) {
	if (index >= m_monitors.size())
		return;

	stopCapture(true);

	try {
		if (!r_capture->startCaptureMonitor(m_monitors[index].getHandle(), true)) {
			reportCaptureError(L"startCaptureMonitor");
			stopCapture();
			return;
		}

		r_capture->askForRefresh();
		nowSource = SourceKind::Monitor;
		nowPlay = -1;
		nowMonitor = static_cast<int>(index);
		m_lastFrame.release();
		m_statusMessage = L"Screen capture started.";
		updateControls();
		InvalidateRect(m_hwnd, NULL, true);
	}
	catch (...) {
		reportCaptureError(L"startCaptureMonitor");
		stopCapture();
	}
}

void MainWindow::showPreview() {
	if (m_lastFrame.empty())
		return;

	try {
		cv::Mat preview;
		if (isSample) {
			const int width = std::max(1, m_lastFrame.cols / kDecimationFactor);
			const int height = std::max(1, m_lastFrame.rows / kDecimationFactor);
			cv::resize(
				m_lastFrame, preview,
				cv::Size(width, height),
				0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST
			);
		}
		else {
			preview = m_lastFrame;
		}

		const int effectiveScale = scale * (isSample ? kDecimationFactor : 1);
		if (effectiveScale != 100) {
			const int width = std::max(1, preview.cols * effectiveScale / 100);
			const int height = std::max(1, preview.rows * effectiveScale / 100);
			cv::resize(
				preview, preview,
				cv::Size(width, height),
				0.0, 0.0, cv::InterpolationFlags::INTER_LINEAR
			);
		}

		cv::imshow("WGC Capture Preview", preview);
		cv::waitKey(1);
	}
	catch (...) {
		m_statusMessage = L"Failed to build the preview.";
	}
}

void MainWindow::saveFrame(bool bgr) {
	if (m_lastFrame.empty()) {
		MessageBoxW(
			m_hwnd,
			L"No captured frame is available yet.",
			L"WARNING", MB_ICONWARNING
		);
		return;
	}

	WCHAR path[MAX_PATH] = {};
	swprintf_s(
		path,
		L"capture_%04llu_%s.png",
		static_cast<unsigned long long>(saveCount),
		bgr ? L"bgr" : L"bgra"
	);

	const wchar_t filter[] =
		L"PNG image (*.png)\0*.png\0All files (*.*)\0*.*\0\0";
	OPENFILENAMEW dialog = {};
	dialog.lStructSize = sizeof(dialog);
	dialog.hwndOwner = m_hwnd;
	dialog.lpstrFilter = filter;
	dialog.lpstrFile = path;
	dialog.nMaxFile = MAX_PATH;
	dialog.lpstrDefExt = L"png";
	dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (!GetSaveFileNameW(&dialog))
		return;

	try {
		std::filesystem::path file(path);
		if (_wcsicmp(file.extension().c_str(), L".png") != 0)
			file.replace_extension(L".png");

		cv::Mat image;
		if (bgr)
			cv::cvtColor(
				m_lastFrame, image,
				cv::ColorConversionCodes::COLOR_BGRA2BGR
			);
		else
			image = m_lastFrame;

		std::vector<uchar> encoded;
		if (!cv::imencode(".png", image, encoded))
			throw std::runtime_error("imencode failed");

		std::ofstream output(file.wstring().c_str(), std::ios::binary);
		if (!output)
			throw std::runtime_error("cannot open output file");

		output.write(
			reinterpret_cast<const char*>(encoded.data()),
			static_cast<std::streamsize>(encoded.size())
		);
		if (!output)
			throw std::runtime_error("cannot write output file");

		++saveCount;
		m_statusMessage =
			std::wstring(L"Saved full-resolution ") +
			(bgr ? L"3-channel BGR: " : L"4-channel BGRA: ") +
			file.wstring();
		updateControls();
		InvalidateRect(m_hwnd, NULL, true);
	}
	catch (std::exception const&) {
		MessageBoxW(
			m_hwnd,
			L"Failed to save the captured frame.",
			L"ERROR", MB_ICONERROR
		);
	}
}

void MainWindow::updateControls() {
	if (!hButtonSave || !hButtonSaveC3)
		return;

	const bool hasFrame = !m_lastFrame.empty();
	Button_Enable(hButtonSave, hasFrame);
	Button_Enable(hButtonSaveC3, hasFrame);
	Button_Enable(hButtonScreen, !m_monitors.empty());
}

void MainWindow::reportCaptureError(const wchar_t* action) {
	WCHAR message[1024] = {};
	swprintf_s(
		message,
		L"%s failed.\n\nHRESULT: 0x%08X\n%s",
		action,
		static_cast<unsigned int>(wgc::getLastError()),
		wgc::getLastErrorMessage()
	);
	MessageBoxW(m_hwnd, message, L"Capture error", MB_ICONERROR);
	m_statusMessage = std::wstring(action) + L" failed.";
}

std::wstring MainWindow::makeMonitorLabel(size_t index) const {
	if (index >= m_monitors.size())
		return {};

	const auto& monitor = m_monitors[index];
	std::wstring label = monitor.getDeviceName();
	if (label.empty())
		label = L"Monitor " + std::to_wstring(index + 1);

	label += L" (" + std::to_wstring(monitor.getWidth()) +
		L"x" + std::to_wstring(monitor.getHeight()) + L")";
	if (monitor.isPrimary())
		label += L" [Primary]";
	return label;
}

void MainWindow::stopCapture(bool special) {
	nowSource = SourceKind::None;
	nowPlay = -1;
	nowMonitor = -1;

	if (r_capture)
		r_capture->stopCapture();

	if (!special) {
		SendMessageW(hComboBoxHwnd, CB_SETCURSEL, -1, 0);
		SendMessageW(hMonitorCombo, CB_SETCURSEL, -1, 0);
	}

	cv::destroyAllWindows();
	m_statusMessage = L"Capture stopped.";
	updateControls();

	if (!special)
		InvalidateRect(m_hwnd, NULL, true);
}

} // namespace ohms
