#pragma once

#ifdef WGCCAPTUREWITHOPENCV_EXPORTS
#define WGCCAPTUREWITHOPENCV_API __declspec(dllexport)
#else
#define WGCCAPTUREWITHOPENCV_API __declspec(dllimport)
#endif

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

/**
 * @brief 获取 截图控制器 实例。
 * @brief Get the instance of Capture Manager.
 * @return a pointer to the instance. It may be nullptr.
*/
WGCCAPTUREWITHOPENCV_API ICapture* getInstance();
/**
 * @brief 销毁 截图控制器 实例，并清理现场。
 * @brief Destroy the instance of Capture Manager and clear things about it.
*/
WGCCAPTUREWITHOPENCV_API void drop();

} // namespace wgc
} // namespace ohms
