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
#include "WndEnumeration.h"

namespace {

std::wstring myGetClassName(HWND hwnd) {
	std::array<WCHAR, 1024> className{};

	const int length = ::GetClassNameW(
		hwnd, className.data(), static_cast<int>(className.size())
	);
	if (length <= 0)
		return {};

	return std::wstring(className.data(), static_cast<size_t>(length));
}


std::wstring myGetWindowText(HWND hwnd) {
	std::array<WCHAR, 1024> windowText{};

	const int length = ::GetWindowTextW(
		hwnd, windowText.data(), static_cast<int>(windowText.size())
	);
	if (length <= 0)
		return {};

	return std::wstring(windowText.data(), static_cast<size_t>(length));
}


bool isAltTabWindow(ohms::WindowInfo const& window) noexcept {
	HWND hwnd = window.getHWnd();

	std::wstring title = window.getTitle();
	std::wstring className = window.getClassName();

	if (hwnd == GetShellWindow()) {
		return false;
	}

	if (title.length() == 0) {
		return false;
	}

	if (!IsWindowVisible(hwnd)) {
		return false;
	}

	if (GetAncestor(hwnd, GA_ROOT) != hwnd) {
		return false;
	}

	const LONG style = GetWindowLongW(hwnd, GWL_STYLE);
	if (style & WS_DISABLED) {
		return false;
	}

	DWORD cloaked = FALSE;
	const HRESULT hrTemp = DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));

	if (SUCCEEDED(hrTemp) && cloaked == DWM_CLOAKED_SHELL) {
		return false;
	}
	return true;
}


BOOL CALLBACK myEnumWindowsProc(HWND hwnd, LPARAM lParam) {
	std::wstring class_name = myGetClassName(hwnd);
	std::wstring title = myGetWindowText(hwnd);

	ohms::WindowInfo window = ohms::WindowInfo(hwnd, title, class_name);

	if (!isAltTabWindow(window)) {
		return TRUE;
	}

	std::vector<ohms::WindowInfo>& windows =
		*reinterpret_cast<std::vector<ohms::WindowInfo>*>(lParam);
	windows.push_back(window);

	return TRUE;
}

} // namespace

namespace ohms {

WindowInfo::WindowInfo() noexcept :
	m_hwnd(NULL) {}

WindowInfo::WindowInfo(HWND hwnd, std::wstring const& title, std::wstring const& className) {
	m_hwnd = hwnd;
	m_title = title;
	m_className = className;
}

HWND WindowInfo::getHWnd() const noexcept {
	return m_hwnd;
}

std::wstring WindowInfo::getTitle() const {
	return m_title;
}

std::wstring WindowInfo::getClassName() const {
	return m_className;
}

void EnumerateWindows(std::vector<WindowInfo>& windows) {
	EnumWindows(::myEnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
}

}
