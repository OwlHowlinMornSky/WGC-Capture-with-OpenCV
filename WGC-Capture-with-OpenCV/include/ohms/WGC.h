#pragma once

// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 WGCCAPTUREWITHOPENCV_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// WGCCAPTUREWITHOPENCV_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef WGCCAPTUREWITHOPENCV_EXPORTS
#define WGCCAPTUREWITHOPENCV_API __declspec(dllexport)
#else
#define WGCCAPTUREWITHOPENCV_API __declspec(dllimport)
#endif

#include <opencv2/core/mat.hpp>

namespace ohms {
namespace wgc {

/**
 * @brief 截图组件控制器 的 接口。
*/
class WGCCAPTUREWITHOPENCV_API ICapture {
protected:
	ICapture() = default;
public:
	virtual ~ICapture() = default;

public:
	/**
	 * @brief 创建对指定窗口的截取。
	 * @param hwnd: 指定窗口。
	 * @return 是否成功。
	*/
	virtual bool startCapture(HWND hwnd) = 0;
	/**
	 * @brief 停止截取。
	*/
	virtual void stopCapture() = 0;

	/**
	 * @brief 设置截取客户区。
	 * @param enabled: 是否截取客户区。
	*/
	virtual void setClipToClientArea(bool enabled) = 0;
	/**
	 * @brief 是否已设为截取客户区。
	 * @return true 则 已设为截取客户区。
	*/
	virtual bool isClipToClientArea() = 0;

	/**
	 * @brief 令截取器在下次捕获到帧时刷新 cv::Mat。
	*/
	virtual void askForRefresh() = 0;
	/**
	 * @brief cv::Mat 是否已经刷新。
	 * @return true 则从上次 askForRefresh 后已经刷新。
	*/
	virtual bool isRefreshed() = 0;

	/**
	 * @brief 获取当前 cv::Mat。
	 * @param target: 保存 cv::Mat 的地方。
	 * @return 操作是否成功。
	*/
	virtual bool copyMat(cv::Mat& target, bool convertToBGR = false) = 0;
};

/**
 * @brief 获取 截图控制器 实例。
 * @return 指向实例的指针。可能为 nullptr。
*/
WGCCAPTUREWITHOPENCV_API ICapture* getInstance();
/**
 * @brief 销毁 截图控制器 实例，并清理现场。
*/
WGCCAPTUREWITHOPENCV_API void drop();

} // namespace wgc
} // namespace ohms
