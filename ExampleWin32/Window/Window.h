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

#include "../framework.h"

namespace ohms {

class Window {
public:
	static Window* CreateMainWindow();

public:
protected:
	Window();

public:
	virtual ~Window() noexcept;

public:
	/**
	 * @brief 窗口是否开启（已创建 且 未销毁）。
	 * @return true 则 仍开启。
	*/
	bool isOpen() const noexcept;

	/**
	 * @brief 创建窗口。
	 * @param nShowCmd: 显示代码。
	 * @return 创建是否成功。
	*/
	virtual bool create(int nShowCmd = SW_NORMAL) noexcept;
	/**
	 * @brief 销毁。
	*/
	virtual void destroy() noexcept;

public:
	/**
	 * @brief 窗口过程。
	 * @param hwnd: 窗口句柄。
	 * @param msg: 消息代码。
	 * @param wp: w参数。
	 * @param lp: l参数。
	 * @return 返回值。
	*/
	virtual LRESULT procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

protected:
	HWND m_hwnd; // 本窗口句柄。
};

} // namespace ohms
