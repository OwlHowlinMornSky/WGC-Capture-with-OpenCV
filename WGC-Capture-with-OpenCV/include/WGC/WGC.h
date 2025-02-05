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

#ifdef WGCCAPTUREWITHOPENCV_EXPORTS
#define WGCCAPTUREWITHOPENCV_API __declspec(dllexport)
#else
#define WGCCAPTUREWITHOPENCV_API __declspec(dllimport)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <opencv2/core/mat.hpp>
#include <memory>

#include <functional>

namespace wgc {

class ICapturer;

/**
 * @brief Interface of Capture Factory.
 * @brief You COULD create multiple instance of this factory.
*/
class WGCCAPTUREWITHOPENCV_API IFactory {
protected:
	IFactory() = default;
public:
	virtual ~IFactory() = default;

public:
	/**
	 * @brief Create an instance of Capture Factory.
	 * @brief This function may throws.
	 * @param com_initialized: Set true if COM is initialized. 
	 * @return A pointer to the instance.
	 */
	static std::shared_ptr<IFactory> createInstance(bool com_initialized);

	/**
	 * @brief Create an instance of Capture Factory.
	 * @param com_initialized: Set true if COM is initialized. 
	 * @return A pointer to the instance. It may be nullptr if failed.
	 */
	static std::shared_ptr<IFactory> createInstanceNoThrow(bool com_initialized) noexcept;

public:
	/**
	 * @brief This function may throws.
	 */
	virtual std::weak_ptr<ICapturer> createCapturer() = 0;
	/**
	 * @brief This function may throws.
	 */
	virtual void destroyCapturer(std::weak_ptr<ICapturer> instance) = 0;
};

/**
 * @brief Interface of Capturer.
 * @brief Each factory COULD create multiple capturers.
*/
class WGCCAPTUREWITHOPENCV_API ICapturer {
protected:
	ICapturer() = default;
public:
	virtual ~ICapturer() = default;

public:
	/**
	 * @brief Start capture the specified window.
	 * @brief If it's capturing, it stops previous work and starts to new target.
	 * @brief This function may throws.
	 * @param hwnd: The specified window.
	 * @param freeThreaded: Get frame on another thread or not.
	 * @return 'true' if succeed.
	*/
	virtual bool startCaptureWindow(HWND hwnd, bool freeThreaded = true) = 0;
	/**
	 * @brief Start capture the specified monitor.
	 * @brief If it's capturing, it stops previous work and starts to new target.
	 * @brief This function may throws.
	 * @param hwnd: The specified monitor.
	 * @param freeThreaded: Get frame on another thread or not.
	 * @return 'true' if succeed.
	*/
	virtual bool startCaptureMonitor(HMONITOR hmonitor, bool freeThreaded = true) = 0;

	/**
	 * @brief Start capture the specified window. Get cv::Mat by callback function on another thread.
	 * @brief If it's capturing, it stops previous work and starts to new target.
	 * @brief Notice: askForRefresh(), isRefreshed() will be useless, and copyMatTo() will be non-thread-safe.
	 * @brief This function may throws.
	 * @param hwnd: The specified window.
	 * @param cb: The callback function.
	 * @return 'true' if succeed.
	*/
	virtual bool startCaptureWindowWithCallback(HWND hwnd, std::function<void(const cv::Mat&)> cb) = 0;
	/**
	 * @brief Start capture the specified monitor. Get cv::Mat by callback function on another thread.
	 * @brief If it's capturing, it stops previous work and starts to new target.
	 * @brief Notice: askForRefresh(), isRefreshed() will be useless, and copyMatTo() will be non-thread-safe.
	 * @brief This function may throws.
	 * @param hwnd: The specified monitor.
	 * @param cb: The callback function.
	 * @return 'true' if succeed.
	*/
	virtual bool startCaptureMonitorWithCallback(HMONITOR hmonitor, std::function<void(const cv::Mat&)> cb) = 0;

	/**
	 * @brief Stop capture. Nothing will happend if it's not capturing.
	 * @brief This function may throws.
	*/
	virtual void stopCapture() = 0;

	/**
	 * @brief Set clipping to the client area of the target window or not.
	 * @brief If capturing a monitor, this keeps no change.
	 * @param enabled: give 'true' if only the client area is wanted.
	*/
	virtual void setClipToClientArea(bool enabled) = 0;
	/**
	 * @brief Query that if it is clipping to the client area or not.
	 * @return 'true' if clipping.
	*/
	virtual bool isClipToClientArea() = 0;

	/**
	 * @brief Query if it is capturing.
	 * @return 'true' if capturing.
	 */
	virtual bool isCapturing() = 0;
	/**
	 * @brief Query if it has been set to capturing a window.
	 * @return 'true' if a window now is captured.
	*/
	virtual bool isCaptureWindow() = 0;
	/**
	 * @brief Query if it has been set to capturing a monitor.
	 * @return 'true' if a monitor now is captured.
	*/
	virtual bool isCaptureMonitor() = 0;

	/**
	 * @brief Require one new frame into the internal cv::Mat.
	 * @brief Frames will be discarded if this function is not called or a frame already arrived before.
	*/
	virtual void askForRefresh() = 0;
	/**
	 * @brief Query if the internal cv::Mat has been refreshed after the last call of askForRefresh().
	 * @return 'true' if the internal cv::Mat has been refershed。
	*/
	virtual bool isRefreshed() = 0;

	/**
	 * @brief Get the internal cv::Mat.
	 * @param target: Where the cv::Mat will copy to.
	 * @param convertToBGR: Convert the mat to BGR or it will keep as BGRA.
	 * @return 'true' if success.
	*/
	virtual void copyMatTo(cv::Mat& target, bool convertToBGR = false) = 0;

	/**
	 * @brief Every instance of capturer have an unique id to others.
	 * @brief This is used by factory to clean.
	 */
	virtual size_t getId() const = 0;
};

} // namespace wgc
