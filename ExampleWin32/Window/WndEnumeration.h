#pragma once

#include <dwmapi.h>
#include <array>
#include <vector>
#include <string>

namespace ohms {

struct WindowInfo final {
public:
	WindowInfo() noexcept;
	WindowInfo(HWND hwnd, std::wstring const& title, std::wstring& className) noexcept;
	~WindowInfo() = default;

	HWND getHWnd() const noexcept;
	std::wstring getTitle() const;
	std::wstring getClassName() const;

protected:
	HWND m_hwnd;
	std::wstring m_title;
	std::wstring m_className;
}; // sturct WindowInfo

void EnumerateWindows(std::vector<WindowInfo>& windows) noexcept;

}
