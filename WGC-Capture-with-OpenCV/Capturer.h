/*
*    WGC-Capture-with-OpenCV
*
*     Copyright 2023-2025  Tyler Parret True
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

#include "pch.h"

#include <mutex>
#include <atomic>
#include <functional>
#include "include/WGC/WGC.h"

namespace wgc {

/**
 * @brief 截取器。
*/
class Capturer final :
	public ICapturer {
public:
	Capturer(winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device, size_t id);

	~Capturer();

public:
	virtual bool startCaptureWindow(HWND hwnd, bool freeThreaded = true) override;
	virtual bool startCaptureMonitor(HMONITOR hmonitor, bool freeThreaded = true) override;

	virtual bool startCaptureWindowWithCallback(HWND hwnd, std::function<void(const cv::Mat&)> cb) override;
	virtual bool startCaptureMonitorWithCallback(HMONITOR hmonitor, std::function<void(const cv::Mat&)> cb) override;

	virtual void stopCapture() override;

	virtual void setClipToClientArea(bool enabled) override;
	virtual bool isClipToClientArea() override;

	virtual bool isCapturing() override;
	virtual bool isCaptureMonitor() override;

	virtual void askForRefresh() override;
	virtual bool isRefreshed() override;

	virtual void copyMatTo(cv::Mat& target, bool convertToBGR = false) override;

	virtual size_t getId() const override;

protected:
	void OnFrameArrived(
		winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
		winrt::Windows::Foundation::IInspectable const& args
	);
	void OnFrameArrivedWithCallback(
		winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
		winrt::Windows::Foundation::IInspectable const& args
	); // 注意这个方法不锁mat，不管needUpdate，也不设置updated。
	void CreateTexture();

protected:
	size_t m_id;

	winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item;
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool;
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session;

	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device;
	winrt::com_ptr<ID3D11DeviceContext> m_d3dContext;
	winrt::com_ptr<ID3D11Device> m_d3dDevice;
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrived;

	ID3D11Texture2D* m_texture;
	winrt::Windows::Graphics::SizeInt32 m_lastSize;
	winrt::Windows::Graphics::SizeInt32 m_lastTexSize;

	std::atomic<bool> m_img_clientarea; // 应当由Capture确保 在截取显示器时 不会为true。
	std::atomic<bool> m_img_needRefresh;
	std::atomic<bool> m_img_updated;
	std::function<void(const cv::Mat&)> m_callback;

	D3D11_BOX m_client_box;
	HWND m_target_window;
	HMONITOR m_target_monitor;

	cv::Mat m_cap;
	std::mutex m_mutex_cap;
	std::mutex m_mutex_proc;
};

} // namespace wgc
