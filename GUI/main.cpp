#include <SDL.hpp>
#include <SDL_mixer.hpp>
#include <iostream>
#include <memory>

//#define DEBUG_GUI_RENDER
//#define DEBUG_GUI_CONTAINERS
#include "GUIElements.hpp"

void Program(int argc, char* argv[], SDL::Window& w, SDL::Renderer& r)
{
	using namespace SDL;

	bool running = true;
	Point size = w.GetSize();

	GUI::ContainerGroup root
	(
		{
			{0.f, 0.f}, {0.f, 0.f},
			{1.f, 1.f}, {0.f, 0.f}
		}
	);

	root.AddChild
	(
		std::shared_ptr<GUI::BorderedFilledRect>
		(
			new GUI::BorderedFilledRect
			(
				0,
				r,
				{
					{ 0.f, 0.f }, {  35.f, 35.f },
					{ 1.f, 0.f }, { -70.f, 20.f }
				},
				SDL::VERY_DARK_GREY,
				SDL::LIGHT_GREY
			)
		)
	);

	root.AddChild
	(
		std::shared_ptr<GUI::BorderedFilledRect>
		(
			new GUI::BorderedFilledRect
			(
				0,
				r,
				{
					{ 0.f, 0.f }, { 35.f, 90.f },
					{ 0.f, 0.f }, { 50.f, 20.f }
				},
				SDL::VERY_DARK_GREY,
				SDL::LIGHT_GREY
			)
		)
	);

	auto volume = new GUI::FloatSlider
	(
		r,
		{
			{ 0.f, 0.f }, {  35.f, 35.f },
			{ 1.f, 0.f }, { -70.f, 20.f }
		},
		{
			{  0.f, 0.5f },
			{ 10.f, 0.0f }
		},
		{
			{   1.f, 0.5f },
			{ -10.f, 0.0f }
		},
		{
			{ 0.f, 0.f }, { -9.f, -9.f },
			{ 0.f, 0.f }, { 18.f, 18.f }
		},
		0.f, 1.f, .5f,
		Button::LEFT
	);

	volume->AddChild
	(
		std::shared_ptr<GUI::FilledRect>
		(
			new GUI::FilledRect
			(
				0,
				r,
				{
					{ 0.f, 0.f }, { -9.f, -9.f },
					{ 0.f, 0.f }, { 18.f, 18.f }
				},
				SDL::GREY
			)
		)
	);

	auto toggle = new GUI::Toggle
	(
		r,
		{
			{ 0.f, 0.f }, { 35.f, 90.f },
			{ 0.f, 0.f }, { 50.f, 20.f }
		},
		{
			{ 0.f, 0.5f }, { 10.f,  0.f }
		},
		{
			{ 1.f, 0.5f }, { -10.f,  0.f }
		},
		{
			{ 0.f, 0.f }, { 0.f, 0.f },
			{ 1.f, 1.f }, { 0.f, 0.f }
		},
		false,
		50,
		Button::LEFT
	);

	root.AddChild(std::shared_ptr<GUI::FloatSlider>(volume));
	root.AddChild(std::shared_ptr<GUI::Toggle     >(toggle));

	toggle->AddChild
	(
		std::shared_ptr<GUI::FilledRect>
		(
			new GUI::FilledRect
			(
				0,
				r,
				{
					{ 0.f, 0.f }, { -9.f, -9.f },
					{ 0.f, 0.f }, { 18.f, 18.f }
				},
				SDL::GREY
			)
		)
	);

	root.SetParentShape({ { 0.f, 0.f }, size });

	Listener<const Event&> quit_listener
	(
		[&running](const Event& e)->void
		{
			running = false;
		},
		Input::GetTypedEventSubject(Event::Type::QUIT)
	);

	Listener<const Event&> resize_listener
	(
		[&size, &root](const Event& e)->void
		{
			if (e.window.event != SDL_WINDOWEVENT_RESIZED) return;

			size.w = e.window.data1;
			size.h = e.window.data2;

			root.SetParentShape({ { 0.f, 0.f }, size });
		},
		Input::GetTypedEventSubject(Event::Type::WINDOWEVENT)
	);

	Uint64 t = GetTicks64();
	Uint64 dT = 0;

	do
	{
		dT = GetTicks64() - t;
		t += dT;

		Input::Update();

		GUI::IUpdateable::UpdateAll(dT);

		r.SetDrawColour(BLACK);
		r.Clear();

		GUI::IRenderable::RenderAllGUI();

#ifdef DEBUG_GUI_CONTAINERS
		GUI::IContainer::RenderAllParents(r);
		GUI::IContainer::RenderAllShapes(r);
		GUI::IContainer::RenderAllAnchors(r);
#endif

		r.Present();
	} while (running);
}

int main(int argc, char* argv[])
{
	using namespace SDL;

	if (!Init(InitFlags::VIDEO | InitFlags::EVENTS | InitFlags::AUDIO))
	{
		return -1;
	}

	if (!Input::Init())
	{
		Quit();
		return -1;
	}

	if (MIX::Init(MIX::InitFlags::MP3 | MIX::InitFlags::OGG) == MIX::InitFlags::NONE)
	{
		Input::Quit();
		Quit();
		return -1;
	}
	
	{
		Window w;
		Renderer r;

		if (CreateWindowAndRenderer({ 300, 300 }, w, r, WindowFlags::SHOWN | WindowFlags::INPUT_FOCUS | WindowFlags::RESIZABLE))
		{
			Program(argc, argv, w, r);
		}
	}

	Input::Quit();
	MIX::Quit;
	Quit();

	return 0;
}