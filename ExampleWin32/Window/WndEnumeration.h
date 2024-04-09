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
