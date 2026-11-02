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

#include <string>
#include <vector>
#include <WGC/WGC.h>

namespace ohms {

class MainWindow final :
	public Window {
public:
	using Parent = Window;

	enum class SourceKind {
		None,
		Window,
		Monitor
	};

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

	void startWindowCapture(size_t index);
	void startMonitorCapture(size_t index);
	void showPreview();
	void saveFrame(bool bgr);
	void updateControls();
	void reportCaptureError(const wchar_t* action);
	std::wstring makeMonitorLabel(size_t index) const;

protected:
	std::shared_ptr<wgc::IFactory> r_factory;
	std::shared_ptr<wgc::ICapturer> r_capture;

	HFONT hFont;
	HWND hComboBoxHwnd;
	HWND hMonitorCombo;
	HWND hButtonRf;
	HWND hButtonStop;
	HWND hButtonSample;
	HWND hButtonBigger;
	HWND hButtonSmaller;
	HWND hButtonSave;
	HWND hButtonSaveC3;
	HWND hButtonSwitchClient;
	HWND hButtonScreen;
	std::vector<ohms::WindowInfo> m_windows;
	std::vector<ohms::MonitorInfo> m_monitors;

	SourceKind nowSource = SourceKind::None;
	bool isSample = false;
	bool isClient = true;
	int scale = 100;
	LONG_PTR nowPlay = -1;
	int nowMonitor = -1;
	size_t saveCount = 0;
	cv::Mat m_lastFrame;
	std::wstring m_statusMessage;
};

} // namespace ohms
