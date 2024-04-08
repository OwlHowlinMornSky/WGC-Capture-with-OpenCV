#include "WndEnumeration.h"

namespace {

std::wstring myGetClassName(HWND hwnd) {
	std::array<WCHAR, 1024> className;

	::GetClassNameW(hwnd, className.data(), static_cast<int>(className.size()));

	std::wstring title(className.data());
	return title;
}


std::wstring myGetWindowText(HWND hwnd) {
	std::array<WCHAR, 1024> windowText;

	::GetWindowTextW(hwnd, windowText.data(), static_cast<int>(windowText.size()));

	std::wstring title(windowText.data());
	return title;
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

	LONG style = GetWindowLongW(hwnd, GWL_STYLE);
	if (!((style & WS_DISABLED) != WS_DISABLED)) {
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

WindowInfo::WindowInfo(HWND hwnd, std::wstring const& title, std::wstring& className) noexcept {
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

void EnumerateWindows(std::vector<WindowInfo>& windows) noexcept {
	EnumWindows(::myEnumWindowsProc, reinterpret_cast<LPARAM>(&windows));
}

}
