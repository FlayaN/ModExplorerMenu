#include "Home.h"
#include "Graphic.h"

namespace Modex
{

	void HomeWindow::Draw()
	{
		ImGui::Text("I had planned on utilizing this page, but for time sake, I'm going to leave it blank for now.");
		ImGui::NewLine();
		ImGui::Text("Check back on future updates!");
	}
	void HomeWindow::Init()
	{
		// Nothing for now.
	}
}
