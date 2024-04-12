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
#include "Capture.h"
#include "CaptureCore.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Capture;

namespace {

inline bool CreateCaptureItemForWindow(winrt::Windows::Graphics::Capture::GraphicsCaptureItem& item, HWND hwnd) {
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	const auto res = interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item));
	return res == S_OK;
}

inline bool CreateCaptureItemForMonitor(winrt::Windows::Graphics::Capture::GraphicsCaptureItem& item, HMONITOR hmonitor) {
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	const auto res = interop_factory->CreateForMonitor(hmonitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item));
	return res == S_OK;
}

inline auto CreateD3DDevice(
	D3D_DRIVER_TYPE const type,
	winrt::com_ptr<ID3D11Device>& device
) {
	WINRT_ASSERT(!device);
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	return D3D11CreateDevice(
		nullptr,
		type,
		nullptr,
		flags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		device.put(),
		nullptr,
		nullptr
	);
}

inline auto CreateD3DDevice() {
	winrt::com_ptr<ID3D11Device> device;
	HRESULT hr = CreateD3DDevice(D3D_DRIVER_TYPE_HARDWARE, device);
	if (DXGI_ERROR_UNSUPPORTED == hr)
		hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
	winrt::check_hresult(hr);
	return device;
}

extern "C" {
	HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(::IDXGIDevice* dxgiDevice,
														   ::IInspectable** graphicsDevice);
}

inline auto CreateDirect3DDevice(IDXGIDevice* dxgi_device) {
	winrt::com_ptr<::IInspectable> d3d_device;
	winrt::check_hresult(CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, d3d_device.put()));
	return d3d_device.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
}

} // namespace 

namespace ohms::wgc {

Capture::Capture() :
	m_device(nullptr),
	m_capture(nullptr),
	m_clipClientArea(false),
	m_isForMonitor(false)

{
	com_ptr<ID3D11Device> d3dDevice = CreateD3DDevice();
	com_ptr<IDXGIDevice> dxgiDevice = d3dDevice.as<IDXGIDevice>();
	m_device = CreateDirect3DDevice(dxgiDevice.get());
}

Capture::~Capture() {
	stopCapture();
	m_capture.reset();
}

bool Capture::startCaptureWindow(HWND hwnd, bool freeThreaded) {
	stopCapture();

	GraphicsCaptureItem item = { nullptr };

	if (!CreateCaptureItemForWindow(item, hwnd)) {
		return false;
	}

	m_capture = std::make_unique<CaptureCore>(m_device, item, hwnd, freeThreaded, false);

	m_isForMonitor = false;

	if (m_clipClientArea)
		m_capture->setClipToClientArea(m_clipClientArea);

	m_capture->Open();

	m_capture->askForRefresh();

	return true;
}

bool Capture::startCaptureMonitor(HMONITOR hmonitor, bool freeThreaded) {
	stopCapture();

	GraphicsCaptureItem item = { nullptr };

	if (!CreateCaptureItemForMonitor(item, hmonitor)) {
		return false;
	}

	m_capture = std::make_unique<CaptureCore>(m_device, item, hmonitor, freeThreaded, false);

	m_isForMonitor = true;

	m_capture->Open();

	m_capture->askForRefresh();

	return true;
}

bool Capture::startCaptureWindowWithCallback(HWND hwnd, std::function<void(const cv::Mat&)> cb) {
	stopCapture();

	GraphicsCaptureItem item = { nullptr };

	if (!CreateCaptureItemForWindow(item, hwnd)) {
		return false;
	}

	m_capture = std::make_unique<CaptureCore>(m_device, item, hwnd, true, true);

	m_isForMonitor = false;

	if (m_clipClientArea)
		m_capture->setClipToClientArea(m_clipClientArea);

	m_capture->SetCallback(cb);

	m_capture->Open();

	m_capture->askForRefresh();

	return true;
}

bool Capture::startCaptureMonitorWithCallback(HMONITOR hmonitor, std::function<void(const cv::Mat&)> cb) {
	stopCapture();

	GraphicsCaptureItem item = { nullptr };

	if (!CreateCaptureItemForMonitor(item, hmonitor)) {
		return false;
	}

	m_capture = std::make_unique<CaptureCore>(m_device, item, hmonitor, true, true);

	m_isForMonitor = true;

	m_capture->SetCallback(cb);

	m_capture->Open();

	m_capture->askForRefresh();

	return true;
}

void Capture::stopCapture() {
	if (m_capture) {
		m_capture->Close();
		m_capture.reset();
	}
	m_isForMonitor = false;
}

void Capture::setClipToClientArea(bool enabled) {
	if (m_clipClientArea == enabled)
		return;
	m_clipClientArea = enabled;
	if (m_isForMonitor) // 显示器不可以裁剪到客户区，故设置无效。
		return;
	if (m_capture)
		m_capture->setClipToClientArea(m_clipClientArea);
}

bool Capture::isClipToClientArea() {
	return !m_isForMonitor && m_clipClientArea;
}

bool Capture::isRunning() {
	return m_capture != nullptr;
}

bool Capture::isCaptureMonitor() {
	return m_isForMonitor;
}

void Capture::askForRefresh() {
	if (m_capture != nullptr)
		m_capture->askForRefresh();
}

bool Capture::isRefreshed() {
	if (m_capture)
		return m_capture->isRefreshed();
	return false;
}

bool Capture::copyMatTo(cv::Mat& target, bool convertToBGR) {
	if (m_capture == nullptr)
		return false;
	m_capture->copyMat(target, convertToBGR);
	return true;
}

} // namespace ohms::wgc
