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
#include "pch.h"
#include "Capturer.h"
#include <windows.graphics.capture.interop.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.System.h>
#include <dwmapi.h>
#include <cstring>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics;
using namespace winrt::Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::DirectX;
using namespace winrt::Windows::Graphics::DirectX::Direct3D11;

namespace {

extern "C" {
	HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(
		IDXGIDevice* dxgiDevice,
		::IInspectable** graphicsDevice
	);
}

inline HRESULT CreateD3DDevice(
	D3D_DRIVER_TYPE const type,
	com_ptr<ID3D11Device>& device
) {
	WINRT_ASSERT(!device);
	constexpr UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	return D3D11CreateDevice(
		nullptr, type, nullptr, flags,
		nullptr, 0, D3D11_SDK_VERSION,
		device.put(), nullptr, nullptr
	);
}

inline auto CreateD3DDevice() {
	com_ptr<ID3D11Device> device;
	HRESULT hr = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE, device);
	if (DXGI_ERROR_UNSUPPORTED == hr)
		hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
	check_hresult(hr);
	return device;
}

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device) {
	com_ptr<::IInspectable> d3d_device;
	check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
	return d3d_device.as<IDirect3DDevice>();
}

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

inline bool CreateCaptureItemForWindow(GraphicsCaptureItem& item, HWND hwnd) {
	auto activation_factory = get_activation_factory<GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	const auto res = interop_factory->CreateForWindow(hwnd, guid_of<IGraphicsCaptureItem>(), put_abi(item));
	return res == S_OK;
}

inline bool CreateCaptureItemForMonitor(GraphicsCaptureItem& item, HMONITOR hmonitor) {
	auto activation_factory = get_activation_factory<GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	const auto res = interop_factory->CreateForMonitor(hmonitor, guid_of<IGraphicsCaptureItem>(), put_abi(item));
	return res == S_OK;
}

} // namespace 

namespace wgc {

Capturer::Capturer(size_t id) :
	m_id(id),

	m_item(nullptr),
	m_framePool(nullptr),
	m_session(nullptr),

	r_device(nullptr),
	r_d3dDevice(nullptr),
	m_d3dContext(nullptr),

	m_texture(nullptr),
	m_lastSize(),
	m_lastTexSize(),

	m_img_clientarea(false),
	m_img_needRefresh(false),
	m_img_updated(false),

	m_client_box(),
	m_active(false),
	m_target_window(NULL),
	m_target_monitor(NULL) {
	r_d3dDevice = CreateD3DDevice();
	com_ptr<IDXGIDevice> dxgiDevice = r_d3dDevice.as<IDXGIDevice>();
	r_device = CreateDirect3DDevice(dxgiDevice.get());
	r_d3dDevice->GetImmediateContext(m_d3dContext.put());
	if (!m_d3dContext)
		throw hresult_error(E_FAIL);
}

Capturer::~Capturer() {
	stopCapture();
}

bool Capturer::startCaptureWindow(HWND hwnd, bool freeThreaded) {
	stopCapture();

	try {
		if (!IsWindow(hwnd) || IsIconic(hwnd))
			return false;

		GraphicsCaptureItem item = { nullptr };
		if (!CreateCaptureItemForWindow(item, hwnd) || !item)
			return false;

		return StartCapture(item, freeThreaded, hwnd, nullptr, {});
	}
	catch (...) {
		stopCapture();
		return false;
	}
}

bool Capturer::startCaptureMonitor(HMONITOR hmonitor, bool freeThreaded) {
	stopCapture();

	try {
		if (hmonitor == nullptr)
			return false;

		GraphicsCaptureItem item = { nullptr };
		if (!CreateCaptureItemForMonitor(item, hmonitor) || !item)
			return false;

		return StartCapture(item, freeThreaded, nullptr, hmonitor, {});
	}
	catch (...) {
		stopCapture();
		return false;
	}
}

bool Capturer::startCaptureWindowWithCallback(HWND hwnd, std::function<void(const cv::Mat&)> cb) {
	stopCapture();

	try {
		if (!cb || !IsWindow(hwnd) || IsIconic(hwnd))
			return false;

		GraphicsCaptureItem item = { nullptr };
		if (!CreateCaptureItemForWindow(item, hwnd) || !item)
			return false;

		return StartCapture(item, true, hwnd, nullptr, std::move(cb));
	}
	catch (...) {
		stopCapture();
		return false;
	}
}

bool Capturer::startCaptureMonitorWithCallback(HMONITOR hmonitor, std::function<void(const cv::Mat&)> cb) {
	stopCapture();

	try {
		if (!cb || hmonitor == nullptr)
			return false;

		GraphicsCaptureItem item = { nullptr };
		if (!CreateCaptureItemForMonitor(item, hmonitor) || !item)
			return false;

		return StartCapture(item, true, nullptr, hmonitor, std::move(cb));
	}
	catch (...) {
		stopCapture();
		return false;
	}
}

bool Capturer::StartCapture(
	GraphicsCaptureItem const& item,
	bool freeThreaded,
	HWND window,
	HMONITOR monitor,
	std::function<void(const cv::Mat&)> callback
) {
	if (!item)
		return false;

	const bool callbackMode = static_cast<bool>(callback);
	Direct3D11CaptureFramePool pool = { nullptr };
	GraphicsCaptureSession session = { nullptr };
	Direct3D11CaptureFramePool::FrameArrived_revoker revoker;

	try {
		if (!GraphicsCaptureSession::IsSupported())
			return false;

		if (!freeThreaded &&
			!winrt::Windows::System::DispatcherQueue::GetForCurrentThread())
			return false;

		const SizeInt32 size = item.Size();
		if (size.Width <= 0 || size.Height <= 0 ||
			size.Width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
			size.Height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
			return false;

		if (freeThreaded)
			pool = Direct3D11CaptureFramePool::CreateFreeThreaded(
				r_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, size
			);
		else
			pool = Direct3D11CaptureFramePool::Create(
				r_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, size
			);
		if (!pool)
			return false;

		session = pool.CreateCaptureSession(item);
		if (!session)
			return false;

		if (winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(
			winrt::hstring(L"Windows.Graphics.Capture.GraphicsCaptureSession"),
			winrt::hstring(L"IsCursorCaptureEnabled")
		)) {
			session.IsCursorCaptureEnabled(false);
		}
		if (winrt::Windows::Foundation::Metadata::ApiInformation::IsPropertyPresent(
			winrt::hstring(L"Windows.Graphics.Capture.GraphicsCaptureSession"),
			winrt::hstring(L"IsBorderRequired")
		)) {
			session.IsBorderRequired(false);
		}

		const auto weak = weak_from_this();
		if (weak.expired())
			return false;

		if (callbackMode) {
			revoker = pool.FrameArrived(
				auto_revoke,
				[weak](
					Direct3D11CaptureFramePool const& sender,
					winrt::Windows::Foundation::IInspectable const& args
				) {
					if (auto self = weak.lock())
						self->OnFrameArrivedWithCallback(sender, args);
				}
			);
		}
		else {
			revoker = pool.FrameArrived(
				auto_revoke,
				[weak](
					Direct3D11CaptureFramePool const& sender,
					winrt::Windows::Foundation::IInspectable const& args
				) {
					if (auto self = weak.lock())
						self->OnFrameArrived(sender, args);
				}
			);
		}

		bool textureReady = false;
		{
			std::lock_guard lockProc(m_mutex_proc);

			m_item = item;
			m_framePool = pool;
			m_session = session;
			m_frameArrived = std::move(revoker);
			m_lastSize = size;
			m_lastTexSize = {};
			m_target_window.store(window);
			m_target_monitor.store(monitor);
			m_callback = std::move(callback);
			m_img_needRefresh.store(false);
			m_img_updated.store(false);

			textureReady = CreateTexture(size);
			if (textureReady)
				m_active.store(true);
		}

		if (!textureReady) {
			stopCapture();
			return false;
		}

		session.StartCapture();
		if (!callbackMode)
			askForRefresh();
		return true;
	}
	catch (...) {
		if (session) {
			try { session.Close(); }
			catch (...) {}
		}
		if (pool) {
			try { pool.Close(); }
			catch (...) {}
		}
		stopCapture();
		return false;
	}
}

void Capturer::stopCapture() {
	std::lock_guard lockProc(m_mutex_proc);
	ResetSessionLocked();
}

void Capturer::ResetSessionLocked() noexcept {
	m_active.store(false);
	m_target_window.store(nullptr);
	m_target_monitor.store(nullptr);

	try {
		m_frameArrived.revoke();
	}
	catch (...) {
	}

	if (m_framePool) {
		try {
			m_framePool.Close();
		}
		catch (...) {
		}
		m_framePool = nullptr;
	}
	if (m_session) {
		try {
			m_session.Close();
		}
		catch (...) {
		}
		m_session = nullptr;
	}
	m_item = nullptr;
	m_texture = nullptr;

	{
		std::lock_guard lock(m_mutex_cap);
		m_cap.release();
	}
	m_img_needRefresh.store(false);
	m_img_updated.store(false);

	m_callback = nullptr;

	m_lastSize = {};
	m_lastTexSize = {};
	m_client_box = {};
}

void Capturer::setClipToClientArea(bool enabled) {
	m_img_clientarea = enabled;
}

bool Capturer::isClipToClientArea() {
	return m_img_clientarea;
}

bool Capturer::isCapturing() {
	return m_active.load();
}

bool Capturer::isCaptureWindow() {
	return m_active.load() && m_target_window.load() != nullptr;
}

bool Capturer::isCaptureMonitor() {
	return m_active.load() && m_target_monitor.load() != nullptr;
}

void Capturer::askForRefresh() {
	m_img_updated.store(false);
	m_img_needRefresh.store(true);
}

bool Capturer::isRefreshed() {
	return m_img_updated.exchange(false);
}

void Capturer::copyMatTo(cv::Mat& target, bool convertToBGR) {
	std::lock_guard<std::mutex> lock(m_mutex_cap);
	if (m_cap.empty()) {
		target.release();
		return;
	}

	if (convertToBGR)
		cv::cvtColor(m_cap, target, cv::ColorConversionCodes::COLOR_BGRA2BGR, 3);
	else
		m_cap.copyTo(target);
}

size_t Capturer::getId() const {
	return m_id;
}

namespace {

bool SamePool(
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& left,
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& right
) {
	if (!left || !right)
		return false;
	return winrt::get_abi(left) == winrt::get_abi(right);
}

} // namespace

void Capturer::OnFrameArrived(
	Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&
) {
	try {
		std::lock_guard lockProc(m_mutex_proc);
		if (!m_active.load())
			return;
		if (!SamePool(m_framePool, sender))
			return;

		const Direct3D11CaptureFrame frame = sender.TryGetNextFrame();
		if (!frame)
			return;
		const SizeInt32 frameContentSize = frame.ContentSize();

		if (m_img_needRefresh.exchange(false)) {
			const auto surface = frame.Surface();
			if (!surface)
				return;
			com_ptr<ID3D11Texture2D> frameSurface =
				::GetDXGIInterfaceFromObject<ID3D11Texture2D>(surface);
			if (!frameSurface)
				return;

			/* need GetDesc because ContentSize is not reliable */
			D3D11_TEXTURE2D_DESC desc = {};
			frameSurface->GetDesc(&desc);

			const HWND target = m_target_window.load();
			bool client_clip_success = false;
			if (m_img_clientarea.load() && target != nullptr &&
				get_client_box(target, desc.Width, desc.Height, &m_client_box)) {
				desc.Width = m_client_box.right - m_client_box.left;
				desc.Height = m_client_box.bottom - m_client_box.top;
				client_clip_success = true;
			}

			const SizeInt32 targetSize{
				static_cast<int32_t>(desc.Width),
				static_cast<int32_t>(desc.Height)
			};
			if (!CreateTexture(targetSize))
				return;

			if (client_clip_success)
				m_d3dContext->CopySubresourceRegion(m_texture.get(), 0, 0, 0, 0, frameSurface.get(), 0, &m_client_box);
			else
				m_d3dContext->CopyResource(m_texture.get(), frameSurface.get());

			if (!UpdateMatFromTexture())
				return;
		}

		if (frameContentSize.Width > 0 && frameContentSize.Height > 0 &&
			(m_lastSize.Width != frameContentSize.Width ||
				m_lastSize.Height != frameContentSize.Height)) {
			m_lastSize = frameContentSize;
			m_framePool.Recreate(r_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
		}
	}
	catch (...) {
	}
}

void Capturer::OnFrameArrivedWithCallback(
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&
) {
	try {
		std::unique_lock lockProc(m_mutex_proc);
		if (!m_active.load())
			return;
		if (!SamePool(m_framePool, sender))
			return;

		const Direct3D11CaptureFrame frame = sender.TryGetNextFrame();
		if (!frame)
			return;
		const SizeInt32 frameContentSize = frame.ContentSize();

		const auto surface = frame.Surface();
		if (!surface)
			return;
		com_ptr<ID3D11Texture2D> frameSurface =
			::GetDXGIInterfaceFromObject<ID3D11Texture2D>(surface);
		if (!frameSurface)
			return;

		/* need GetDesc because ContentSize is not reliable */
		D3D11_TEXTURE2D_DESC desc = {};
		frameSurface->GetDesc(&desc);

		const HWND target = m_target_window.load();
		bool client_clip_success = false;
		if (m_img_clientarea.load() && target != nullptr &&
			get_client_box(target, desc.Width, desc.Height, &m_client_box)) {
			desc.Width = m_client_box.right - m_client_box.left;
			desc.Height = m_client_box.bottom - m_client_box.top;
			client_clip_success = true;
		}

		const SizeInt32 targetSize{
			static_cast<int32_t>(desc.Width),
			static_cast<int32_t>(desc.Height)
		};
		if (!CreateTexture(targetSize))
			return;

		if (client_clip_success)
			m_d3dContext->CopySubresourceRegion(m_texture.get(), 0, 0, 0, 0, frameSurface.get(), 0, &m_client_box);
		else
			m_d3dContext->CopyResource(m_texture.get(), frameSurface.get());

		if (!UpdateMatFromTexture())
			return;

		if (frameContentSize.Width > 0 && frameContentSize.Height > 0 &&
			(m_lastSize.Width != frameContentSize.Width ||
				m_lastSize.Height != frameContentSize.Height)) {
			m_lastSize = frameContentSize;
			m_framePool.Recreate(r_device, DirectXPixelFormat::B8G8R8A8UIntNormalized, 2, m_lastSize);
		}

		cv::Mat frameCopy;
		std::function<void(const cv::Mat&)> callback;
		{
			std::lock_guard lockCap(m_mutex_cap);
			frameCopy = m_cap;
			callback = m_callback;
		}

		lockProc.unlock();
		if (callback)
			callback(frameCopy);
	}
	catch (...) {
	}
}

bool Capturer::CreateTexture(SizeInt32 size) {
	if (!r_d3dDevice || !m_d3dContext)
		return false;
	if (size.Width <= 0 || size.Height <= 0)
		return false;
	if (size.Width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
		size.Height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
		return false;
	if (m_texture &&
		m_lastTexSize.Width == size.Width &&
		m_lastTexSize.Height == size.Height)
		return true;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc = { 1,0 };
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;
	desc.Width = static_cast<UINT>(size.Width);
	desc.Height = static_cast<UINT>(size.Height);

	com_ptr<ID3D11Texture2D> texture;
	const HRESULT hr = r_d3dDevice->CreateTexture2D(&desc, nullptr, texture.put());
	if (FAILED(hr) || !texture)
		return false;

	m_texture = std::move(texture);
	m_lastTexSize = size;
	return true;
}

bool Capturer::UpdateMatFromTexture() {
	if (!m_texture || !m_d3dContext)
		return false;
	if (m_lastTexSize.Width <= 0 || m_lastTexSize.Height <= 0)
		return false;

	const int width = m_lastTexSize.Width;
	const int height = m_lastTexSize.Height;
	const size_t rowBytes = static_cast<size_t>(width) * 4;

	cv::Mat frame(height, width, CV_8UC4);

	D3D11_MAPPED_SUBRESOURCE mappedTex = {};
	const HRESULT hr = m_d3dContext->Map(
		m_texture.get(), 0, D3D11_MAP_READ, 0, &mappedTex
	);
	if (FAILED(hr))
		return false;
	if (mappedTex.pData == nullptr || mappedTex.RowPitch < rowBytes) {
		m_d3dContext->Unmap(m_texture.get(), 0);
		return false;
	}

	const auto* src = static_cast<const uint8_t*>(mappedTex.pData);
	for (int y = 0; y < height; ++y) {
		std::memcpy(
			frame.ptr<uint8_t>(y),
			src + static_cast<size_t>(y) * mappedTex.RowPitch,
			rowBytes
		);
	}
	m_d3dContext->Unmap(m_texture.get(), 0);

	{
		std::lock_guard lock(m_mutex_cap);
		m_cap = std::move(frame);
	}
	m_img_updated.store(true);
	return true;
}

} // namespace wgc
