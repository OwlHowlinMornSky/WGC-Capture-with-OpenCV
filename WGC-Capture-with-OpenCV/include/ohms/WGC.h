﻿/*
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

#ifdef WGCCAPTUREWITHOPENCV_EXPORTS
#define WGCCAPTUREWITHOPENCV_API __declspec(dllexport)
#else
#define WGCCAPTUREWITHOPENCV_API __declspec(dllimport)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <opencv2/core/mat.hpp>

namespace ohms {
namespace wgc {

/**
 * @brief 截图组件控制器的接口。
 * @brief Interface of Capture Manager.
*/
class WGCCAPTUREWITHOPENCV_API ICapture {
protected:
	ICapture() = default;
public:
	virtual ~ICapture() = default;

public:
	/**
	 * @brief 构造 截图控制器 实例，可以指定 WinRT 是否已经初始化。
	 * @brief Construct the instance of Capture Manager explicitly.
	 * @param winrt_initialized: true if WinRT is already initialized.
	 * @return true if succeed.
	*/
	static bool setup(bool winrt_initialized);

	/**
	 * @brief 获取 截图控制器 实例。
	 * @brief Get the instance of Capture Manager.
	 * @return a pointer to the instance. It may be nullptr.
	*/
	static ICapture* getInstance();

	/**
	 * @brief 销毁 截图控制器 实例，并清理现场。
	 * @brief Destroy the instance of Capture Manager and clear things about it.
	*/
	static void drop();

public:
	/**
	 * @brief 开始捕获指定窗口
	 * @brief Start capture the referred window.
	 * @param hwnd: The referred window.
	 * @return true if success.
	*/
	virtual bool startCapture(HWND hwnd) = 0;
	/**
	 * @brief 停止捕获。如果未在捕获则无事发生。
	 * @brief Stop capture. Nothing will happend if it's not capturing.
	*/
	virtual void stopCapture() = 0;

	/**
	 * @brief 设置截取客户区。
	 * @brief Capture the client area of the target or not.
	 * @param enabled: true if only the client area is wanted.
	*/
	virtual void setClipToClientArea(bool enabled) = 0;
	/**
	 * @brief 是否已设为截取客户区。
	 * @brief Ask if it has been set to capturing the client area or not.
	 * @return true if only the client area now is captured.
	*/
	virtual bool isClipToClientArea() = 0;

	/**
	 * @brief 令截取器在下次捕获到帧时刷新 内部cv::Mat。
	 * @brief 不调用此函数则捕获到的帧都会被丢弃。
	 * @brief Require one new frame into the internal cv::Mat.
	 * @brief Frames will be deserted if this function is not called.
	*/
	virtual void askForRefresh() = 0;
	/**
	 * @brief 内部cv::Mat 是否已经刷新。
	 * @brief Ask if the internal cv::Mat has been refreshed after the last call of askForRefresh().
	 * @return true if the internal cv::Mat has been refershed。
	*/
	virtual bool isRefreshed() = 0;

	/**
	 * @brief 获取当前 cv::Mat。
	 * @brief Get the internal cv::Mat.
	 * @param target: Which place the cv::Mat will copy to.
	 * @param convertToBGR: Convert the mat to BGR or it will keep as BGRA.
	 * @return true if success.
	*/
	virtual bool copyMatTo(cv::Mat& target, bool convertToBGR = false) = 0;
};

} // namespace wgc
} // namespace ohms
