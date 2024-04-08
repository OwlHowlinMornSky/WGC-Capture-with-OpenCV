#pragma once

#include "Window.h"
#include "WndEnumeration.h"

#include <vector>
#include <ohms/WGC.h>

namespace ohms {

class MainWindow final :
	public Window {
public:
	using Parent = Window;

public:
	MainWindow();
	virtual ~MainWindow() override;

public:
	virtual bool create(int nShowCmd = SW_NORMAL) noexcept override;
	virtual void destroy() noexcept override;

public:
	virtual LRESULT procedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) noexcept override;

protected:

	void refreshCombox();
	void stopCapture(bool special = false);

protected:
	ohms::wgc::ICapture* g_app;


	HFONT hFont;
	HWND hComboBoxHwnd;
	HWND hButtonRf;
	HWND hButtonStop;
	HWND hButtonSample;
	HWND hButtonBigger;
	HWND hButtonSmaller;
	HWND hButtonSave;
	HWND hButtonSaveC3;
	HWND hButtonSwitchClient;
	std::vector<ohms::WindowInfo> m_windows;

	bool isSample = false;
	bool isClient = true;
	int scale = 100;
	LONG_PTR nowPlay = -1;
	size_t saveCount = 0;
};

} // namespace ohms
