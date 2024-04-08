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
