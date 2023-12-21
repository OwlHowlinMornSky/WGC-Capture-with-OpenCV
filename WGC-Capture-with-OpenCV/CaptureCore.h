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

#include <inspectable.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>
#include <d3d11_4.h>

#include <mutex>
#include <atomic>

namespace ohms::wgc {

/**
 * @brief 截取器。
*/
class CaptureCore final {
public:
	CaptureCore(
		winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device,
		winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item,
		HWND targetWindow
	);

	~CaptureCore();

public:
	/**
	 * @brief 设置截取客户区。
	 * @param enabled: 是否截取客户区。
	*/
	void setClipToClientArea(bool enabled);
	/**
	 * @brief 是否已设为截取客户区。
	 * @return true 则 已设为截取客户区。
	*/
	bool isClipToClientArea();

	/**
	 * @brief 请求刷新 cv::Mat。
	*/
	void askForRefresh();
	/**
	 * @brief 是否已经刷新 cv::Mat。
	 * @return ture 则 已经刷新。
	*/
	bool isRefreshed();
	/**
	 * @brief 获取当前 cv::Mat。
	 * @param target: 保存位置。
	*/
	void copyMat(cv::Mat& target, bool convertToBGR);

public:
	void Open();
	void Close();

protected:
	void OnFrameArrived(
		winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
		winrt::Windows::Foundation::IInspectable const& args
	);
	void CreateTexture();

protected:
	std::atomic_bool m_closed;

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

	std::atomic<bool> m_img_clientarea;
	std::atomic<bool> m_img_needRefresh;
	std::atomic<bool> m_img_updated;

	D3D11_BOX m_client_box;
	HWND m_target_window;

	cv::Mat m_cap;
	std::mutex m_mutex_cap;
};

} // namespace ohms::wgc
