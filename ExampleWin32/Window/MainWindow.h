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
