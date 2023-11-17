#include "pch.h"
#include "Capture.h"

#include <iostream>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Capture;

namespace {

inline bool CreateCaptureItemForWindow(winrt::Windows::Graphics::Capture::GraphicsCaptureItem& item, HWND hwnd) {
	auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
	auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
	const auto res = interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item));
	if (res != S_OK)
		return false;
	return true;
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
	if (DXGI_ERROR_UNSUPPORTED == hr) {
		hr = CreateD3DDevice(D3D_DRIVER_TYPE_WARP, device);
	}
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
	m_clipClientArea(false) {
	com_ptr<ID3D11Device> d3dDevice = CreateD3DDevice();
	com_ptr<IDXGIDevice> dxgiDevice = d3dDevice.as<IDXGIDevice>();
	m_device = CreateDirect3DDevice(dxgiDevice.get());
}

Capture::~Capture() {
	stopCapture();
	m_capture.reset();
}

bool Capture::startCapture(HWND hwnd) {
	stopCapture();

	GraphicsCaptureItem item = { nullptr };

	if (!CreateCaptureItemForWindow(item, hwnd)) {
		return false;
	}

	m_capture = std::make_unique<CaptureCore>(m_device, item, hwnd);
	if (m_clipClientArea)
		m_capture->setClipToClientArea(m_clipClientArea);
	m_capture->Open();
	return true;
}

void Capture::stopCapture() {
	if (m_capture) {
		m_capture->Close();
		m_capture.reset();
	}
}

void Capture::setClipToClientArea(bool enabled) {
	if (m_clipClientArea == enabled)
		return;
	m_clipClientArea = enabled;
	if (m_capture)
		m_capture->setClipToClientArea(m_clipClientArea);
	return;
}

bool Capture::isClipToClientArea() {
	return m_clipClientArea;
}

void Capture::askForRefresh() {
	if (m_capture != nullptr)
		m_capture->askForRefresh();
}

bool Capture::isRefreshed() {
	if (m_capture) {
		return m_capture->isRefreshed();
	}
	return false;
}

bool Capture::copyMat(cv::Mat& target, bool convertToBGR) {
	if (m_capture == nullptr)
		return false;
	m_capture->copyMat(target, convertToBGR);
	return true;
}

} // namespace ohms::wgc
