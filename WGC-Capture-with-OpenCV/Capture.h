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

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <d3d11_4.h>
#include <windows.graphics.capture.interop.h>

#include "include/ohms/WGC.h"
#include "CaptureCore.h"

namespace ohms::wgc {

class Capture final :
	public ICapture {
public:
	Capture();
	virtual ~Capture() override;

public:
	virtual bool startCaptureWindow(HWND hwnd, bool freeThreaded = true) override;
	virtual bool startCaptureMonitor(HMONITOR hmonitor, bool freeThreaded = true) override;
	virtual bool startCaptureWindowWithCallback(HWND hwnd, std::function<void(const cv::Mat&)> cb) override;
	virtual bool startCaptureMonitorWithCallback(HMONITOR hmonitor, std::function<void(const cv::Mat&)> cb) override;
	virtual void stopCapture() override;

	virtual void setClipToClientArea(bool enabled) override;
	virtual bool isClipToClientArea() override;

	virtual bool isRunning() override;
	virtual bool isCaptureMonitor() override;

	virtual void askForRefresh() override;
	virtual bool isRefreshed() override;

	virtual bool copyMatTo(cv::Mat& target, bool convertToBGR = false) override;

protected:
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device;
	std::unique_ptr<CaptureCore> m_capture; // 截取器实例。

	bool m_clipClientArea;
	bool m_isForMonitor;
};

} // namespace ohms::wgc
