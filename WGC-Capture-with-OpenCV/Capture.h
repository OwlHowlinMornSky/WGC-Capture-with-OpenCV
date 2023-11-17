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
	virtual bool startCapture(HWND hwnd) override;
	virtual void stopCapture() override;

	virtual void setClipToClientArea(bool enabled) override;
	virtual bool isClipToClientArea() override;

	virtual void askForRefresh() override;
	virtual bool isRefreshed() override;

	virtual bool copyMatTo(cv::Mat& target, bool convertToBGR = false) override;

protected:
	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device;
	std::unique_ptr<CaptureCore> m_capture; // 截取器实例。

	bool m_clipClientArea;
};

} // namespace ohms::wgc
