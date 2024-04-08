#include "Window.h"
#include "MainWindow.h"

namespace ohms {

Window* ohms::Window::CreateMainWindow() {
	return new MainWindow();
}

}