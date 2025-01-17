#include "TaskWindow.h"

#include "Task.h"

#include "gui/interface/Label.h"
#include "gui/interface/Engine.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/Style.h"

#include "graphics/Graphics.h"

#include "common/tpt-minmax.h"

TaskWindow::TaskWindow(String title_, Task * task_, bool closeOnDone):
	ui::Window(ui::Point(-1, -1), ui::Point(240, 60)),
	task(task_),
	title(title_),
	progress(0),
	done(false),
	closeOnDone(closeOnDone),
	progressStatus("0%")
{

	ui::Label * tempLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), title);
	tempLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	tempLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	tempLabel->SetTextColour(style::Colour::WarningTitle);
	AddComponent(tempLabel);

	statusLabel = new ui::Label(ui::Point(4, 23), ui::Point(Size.X-8, 15), "");
	statusLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	statusLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(statusLabel);

	MakeActiveWindow();

	task->AddTaskListener(this);
	task->Start();
}

void TaskWindow::NotifyStatus(Task * task)
{
	statusLabel->SetText(task->GetStatus());
}

void TaskWindow::NotifyError(Task * task)
{
	new ErrorMessage("Error", task->GetError());
	done = true;
}

void TaskWindow::NotifyDone(Task * task)
{
	if(closeOnDone)
		Exit();
	done = true;
}

void TaskWindow::Exit()
{
	if (CloseActiveWindow())
	{
		SelfDestruct();
	}
}

void TaskWindow::NotifyProgress(Task * task)
{
	progress = task->GetProgress();
	if(progress>-1)
		progressStatus = String::Build(progress, "%");
	else
		progressStatus = "Please wait...";
}

void TaskWindow::OnTick(float dt)
{
	intermediatePos += 1.0f*dt;
	if(intermediatePos>100.0f)
		intermediatePos = 0.0f;
	task->Poll();
	if (done)
		Exit();
}

void TaskWindow::OnDraw()
{
	Graphics * g = GetGraphics();
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);

	g->DrawLine(Position + Vec2{ 0, Size.Y-17 }, Position + Vec2{ Size.X - 1, Size.Y-17 }, 0xFFFFFF_rgb);

	ui::Colour progressBarColour = style::Colour::WarningTitle;

	if(progress!=-1)
	{
		if(progress > 0)
		{
			if(progress > 100)
				progress = 100;
			float size = float(Size.X-4)*(float(progress)/100.0f); // TIL...
			size = std::min(std::max(size, 0.0f), float(Size.X-4));
			g->DrawFilledRect(RectSized(Position + Vec2{ 2, Size.Y-15 }, Vec2{ int(size), 13 }), progressBarColour.NoAlpha());
		}
	} else {
		int size = 40, rsize = 0;
		float position = float(Size.X-4)*(intermediatePos/100.0f);
		if(position + size - 1 > Size.X-4)
		{
			size = (Size.X-4)-int(position)+1;
			rsize = 40-size;
		}
		g->DrawFilledRect(RectSized(Position + Vec2{ 2 + int(position), Size.Y-15 }, Vec2{ size, 13 }), progressBarColour.NoAlpha());
		if(rsize)
		{
			g->DrawFilledRect(RectSized(Position + Vec2{ 2, Size.Y-15 }, Vec2{ rsize, 13 }), progressBarColour.NoAlpha());
		}
	}
	g->BlendText(Position + Vec2{ ((Size.X-(Graphics::TextSize(progressStatus).X - 1))/2), Size.Y-13 }, progressStatus, progress<50 ? 0xFFFFFF_rgb .WithAlpha(255) : 0x000000_rgb .WithAlpha(255));
}

TaskWindow::~TaskWindow() {
	delete task;
}

