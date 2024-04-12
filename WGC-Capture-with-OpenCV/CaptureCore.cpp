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
#include "pch.h"
#include "CaptureCore.h"
#include <dwmapi.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;

namespace {

struct __declspec(uuid("A9B3D012-3DF2-4EE3-B8D1-8695F457D3C1"))
	IDirect3DDxgiInterfaceAccess : ::IUnknown {
	virtual HRESULT __stdcall GetInterface(GUID const& id, void** object) = 0;
};

template <typename T> inline auto GetDXGIInterfaceFromObject(winrt::Windows::Foundation::IInspectable const& object) {
	auto access = object.as<::IDirect3DDxgiInterfaceAccess>();
	winrt::com_ptr<T> result;
	winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
	return result;
}

bool get_client_box(HWND window, uint32_t width, uint32_t height, D3D11_BOX* client_box) {
	RECT client_rect = { 0 };
	RECT window_rect = { 0 };
	POINT upper_left = { 0 };

	/* check iconic (minimized) twice, ABA is very unlikely */
	bool client_box_available =
		!IsIconic(window) &&
		GetClientRect(window, &client_rect) &&
		!IsIconic(window) &&
		(client_rect.right > 0) &&
		(client_rect.bottom > 0) &&
		(DwmGetWindowAttribute(
			window, DWMWA_EXTENDED_FRAME_BOUNDS,
			&window_rect, sizeof(window_rect)
		) == S_OK) &&
		ClientToScreen(window, &upper_left);

	if (client_box_available) {
		const uint32_t left = (upper_left.x > window_rect.left) ? (upper_left.x - window_rect.left) : 0;
		client_box->left = left;

		const uint32_t top = (upper_left.y > window_rect.top) ? (upper_left.y - window_rect.top) : 0;
		client_box->top = top;

		uint32_t texture_width = 1;
		if (width > left)
			texture_width = std::min(width - left, (uint32_t)client_rect.right);

		uint32_t texture_height = 1;
		if (height > top)
			texture_height = std::min(height - top, (uint32_t)client_rect.bottom);

		client_box->right = left + texture_width;
		client_box->bottom = top + texture_height;

		client_box->front = 0;
		client_box->back = 1;

		client_box_available = (client_box->right <= width) && (client_box->bottom <= height);
	}
	return client_box_available;
}

} // namespace 

namespace ohms::wgc {

CaptureCore::CaptureCore(
	IDirect3DDevice const& device,
	GraphicsCaptureItem const& item,
	HWND targetWindow, bool freeThreaded, bool useCallback
) :
	m_closed(false),

	m_item(nullptr),
	m_framePool(nullptr),
	m_session(nullptr),

	m_device(nullptr),
	m_d3dContext(nullptr),
	m_d3dDevice(nullptr),

	m_texture(nullptr),

	m_img_clientarea(false),
	m_img_needRefresh(false),
	m_img_updated(false),

	m_target_window(targetWindow)

{
	m_device = device;
	m_item = item;

	// Set up 
	m_d3dDevice = ::GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
	m_d3dDevice->GetImmediateContext(m_d3dContext.put());

	const SizeInt32 size = m_item.Size();

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
	m_lastSize = size;
	m_lastTexSize = size;

	if (freeThreaded || useCallback)
		m_framePool = Direct3D11CaptureFramePool::CreateFreeThreaded(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
	else
		m_framePool = Direct3D11CaptureFramePool::Create(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);

	m_session = m_framePool.CreateCaptureSession(m_item);
	if (useCallback)
		m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &CaptureCore::OnFrameArrivedWithCallback });
	else
		m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &CaptureCore::OnFrameArrived });

	m_session.IsCursorCaptureEnabled(false);
	m_session.IsBorderRequired(false);

	CreateTexture();
}

CaptureCore::CaptureCore(
	IDirect3DDevice const& device,
	GraphicsCaptureItem const& item,
	HMONITOR targetMonitor, bool freeThreaded, bool useCallback
) :
	m_closed(false),

	m_item(nullptr),
	m_framePool(nullptr),
	m_session(nullptr),

	m_device(nullptr),
	m_d3dContext(nullptr),
	m_d3dDevice(nullptr),

	m_texture(nullptr),

	m_img_clientarea(false),
	m_img_needRefresh(false),
	m_img_updated(false),

	m_target_window(NULL)

{
	m_device = device;
	m_item = item;

	// Set up 
	m_d3dDevice = ::GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
	m_d3dDevice->GetImmediateContext(m_d3dContext.put());

	const SizeInt32 size = m_item.Size();

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
	m_lastSize = size;
	m_lastTexSize = size;

	if (freeThreaded || useCallback)
		m_framePool = Direct3D11CaptureFramePool::CreateFreeThreaded(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
	else
		m_framePool = Direct3D11CaptureFramePool::Create(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);

	m_session = m_framePool.CreateCaptureSession(m_item);
	if (useCallback)
		m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &CaptureCore::OnFrameArrivedWithCallback });
	else
		m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &CaptureCore::OnFrameArrived });

	m_session.IsCursorCaptureEnabled(false);
	m_session.IsBorderRequired(false);

	CreateTexture();
}

CaptureCore::~CaptureCore() {
	Close();
}

void CaptureCore::setClipToClientArea(bool enabled) {
	m_img_clientarea = enabled;
}

bool CaptureCore::isClipToClientArea() {
	return m_img_clientarea;
}

void CaptureCore::askForRefresh() {
	m_img_updated.store(false);
	return m_img_needRefresh.store(true);
}

bool CaptureCore::isRefreshed() {
	bool expected = true;
	if (m_img_updated.compare_exchange_weak(expected, false))
		return true;
	return false;
}

void CaptureCore::copyMat(cv::Mat& target, bool convertToBGR) {
	std::lock_guard<std::mutex> lock(m_mutex_cap);
	if (convertToBGR)
		cv::cvtColor(m_cap, target, cv::ColorConversionCodes::COLOR_BGRA2BGR, 3);
	else
		m_cap.copyTo(target);
}

// Start sending capture frames
void CaptureCore::Open() {
	if (m_closed.load() != true)
		m_session.StartCapture();
}

// Process captured frames
void CaptureCore::Close() {
	bool expected = false;
	if (m_closed.compare_exchange_strong(expected, true)) {
		m_frameArrived.revoke();
		m_framePool.Close();
		m_session.Close();

		m_texture->Release();
		m_texture = nullptr;

		m_framePool = nullptr;
		m_session = nullptr;
		m_item = nullptr;
	}
}

void CaptureCore::SetCallback(std::function<void(const cv::Mat&)> cb) {
	m_callback = cb;
}

void CaptureCore::OnFrameArrived(
	Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&
) {
	const Direct3D11CaptureFrame frame = sender.TryGetNextFrame();
	const SizeInt32 frameContentSize = frame.ContentSize();

	bool expected = true;
	if (m_img_needRefresh.compare_exchange_weak(expected, false)) {
		com_ptr<ID3D11Texture2D> frameSurface = ::GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

		/* need GetDesc because ContentSize is not reliable */
		D3D11_TEXTURE2D_DESC desc;
		frameSurface->GetDesc(&desc);

		bool client_clip_success = false;
		if (m_img_clientarea && get_client_box(m_target_window, desc.Width, desc.Height, &m_client_box)) {
			desc.Width = m_client_box.right - m_client_box.left;
			desc.Height = m_client_box.bottom - m_client_box.top;
			client_clip_success = true;
		}

		if (m_lastTexSize.Width != desc.Width || m_lastTexSize.Height != desc.Height) {
			m_lastTexSize.Width = desc.Width;
			m_lastTexSize.Height = desc.Height;
			m_texture->Release();
			CreateTexture();
		}

		if (client_clip_success)
			m_d3dContext->CopySubresourceRegion(m_texture, 0, 0, 0, 0, frameSurface.get(), 0, &m_client_box);
		else
			m_d3dContext->CopyResource(m_texture, frameSurface.get());

		D3D11_MAPPED_SUBRESOURCE mappedTex;
		m_d3dContext->Map(m_texture, 0, D3D11_MAP_READ, 0, &mappedTex);
		m_d3dContext->Unmap(m_texture, 0);
		{
			std::lock_guard<std::mutex> lock(m_mutex_cap);
			m_cap = cv::Mat(
				m_lastTexSize.Height, m_lastTexSize.Width,
				CV_8UC4, mappedTex.pData, mappedTex.RowPitch
			);
		}
		m_img_updated.store(true);
	}

	if (m_lastSize.Width != frameContentSize.Width ||
		m_lastSize.Height != frameContentSize.Height) {
		m_lastSize = frameContentSize;
		m_framePool.Recreate(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
	}
}

void CaptureCore::OnFrameArrivedWithCallback(
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const& args
) {
	const Direct3D11CaptureFrame frame = sender.TryGetNextFrame();
	const SizeInt32 frameContentSize = frame.ContentSize();

	com_ptr<ID3D11Texture2D> frameSurface = ::GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

	/* need GetDesc because ContentSize is not reliable */
	D3D11_TEXTURE2D_DESC desc;
	frameSurface->GetDesc(&desc);

	bool client_clip_success = false;
	if (m_img_clientarea && get_client_box(m_target_window, desc.Width, desc.Height, &m_client_box)) {
		desc.Width = m_client_box.right - m_client_box.left;
		desc.Height = m_client_box.bottom - m_client_box.top;
		client_clip_success = true;
	}

	if (m_lastTexSize.Width != desc.Width || m_lastTexSize.Height != desc.Height) {
		m_lastTexSize.Width = desc.Width;
		m_lastTexSize.Height = desc.Height;
		m_texture->Release();
		CreateTexture();
	}

	if (client_clip_success)
		m_d3dContext->CopySubresourceRegion(m_texture, 0, 0, 0, 0, frameSurface.get(), 0, &m_client_box);
	else
		m_d3dContext->CopyResource(m_texture, frameSurface.get());

	D3D11_MAPPED_SUBRESOURCE mappedTex;
	m_d3dContext->Map(m_texture, 0, D3D11_MAP_READ, 0, &mappedTex);
	m_d3dContext->Unmap(m_texture, 0);
	m_cap = cv::Mat(
		m_lastTexSize.Height, m_lastTexSize.Width,
		CV_8UC4, mappedTex.pData, mappedTex.RowPitch
	);

	m_callback(m_cap);

	if (m_lastSize.Width != frameContentSize.Width ||
		m_lastSize.Height != frameContentSize.Height) {
		m_lastSize = frameContentSize;
		m_framePool.Recreate(m_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
	}
}

inline void CaptureCore::CreateTexture() {
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc = { 1,0 };
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;
	desc.Width = m_lastTexSize.Width;
	desc.Height = m_lastTexSize.Height;

	m_d3dDevice.get()->CreateTexture2D(&desc, nullptr, &m_texture);
}

} // namespace ohms::wgc
