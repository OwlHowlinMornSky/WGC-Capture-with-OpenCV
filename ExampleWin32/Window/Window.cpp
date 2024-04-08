#include "Window.h"

namespace {

const WCHAR g_clsName[] = L"OHMS.WNDCLS"; // 默认窗口类名

/**
 * @brief 全局窗口过程
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (hwnd) {
		if (msg == WM_CREATE) { // 创建窗口时注册相应实例
			LONG_PTR window = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, window);
		}
		ohms::Window* window = reinterpret_cast<ohms::Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)); // 获取相应实例
		if (window) // 获取到实例则调用它的过程函数
			return window->procedure(hwnd, msg, wParam, lParam);
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool g_clsRegistered{ false }; // 是否已注册窗口类的标记

/**
 * @brief 注册窗口类
 * @return true则已经注册
*/
bool registerClass() noexcept {
	if (g_clsRegistered)
		return true;
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandleW(NULL);
	wcex.hIcon = LoadIconW(GetModuleHandleW(NULL), IDI_APPLICATION);
	wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = g_clsName;
	wcex.hIconSm = LoadIconW(wcex.hInstance, IDI_APPLICATION);
	auto res = RegisterClassExW(&wcex);
	if (res != 0)
		g_clsRegistered = true;
	return g_clsRegistered;
}

} // namespace

namespace ohms {

Window::Window() :
	m_hwnd(NULL) {
	return;
}

Window::~Window() {
	destroy();
	return;
}

bool Window::isOpen() const noexcept {
	return m_hwnd != NULL;
}

bool Window::create(int nShowCmd) noexcept {
	if (m_hwnd != NULL || !::registerClass())
		return false; // 窗口类没有注册就不能创建窗口
	m_hwnd = CreateWindowExW(
		0L, g_clsName, L"",
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, NULL, GetModuleHandleW(NULL), this
	);
	if (m_hwnd == NULL)
		return false; // 创建失败
	ShowWindow(m_hwnd, nShowCmd);
	UpdateWindow(m_hwnd);
	return true;
}

void Window::destroy() noexcept {
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
	return;
}

LRESULT Window::procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace ohms
