#include "GUI.hpp"
#include "Lerp.hpp"

namespace GUI
{
	struct FilledRect : public IRenderable
	{
		SDL::Renderer& r;

		SDL::Colour fill_colour;

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

#ifdef DEBUG_GUI_CONTAINERS
			IRenderable::SetParentShape(parent);
#endif
		}

		void RenderGUI()
		{
			r.SetDrawColour(fill_colour);
			r.FillRectF(_shape);
		}

		inline FilledRect(int render_order, SDL::Renderer& r, const GUIRect& shape, SDL::Colour colour)
			: r(r), fill_colour(colour), IRenderable(shape, render_order) {}

	private:
		SDL::FRect _shape;
	};

	struct BorderedRect : public IRenderable
	{
		SDL::Renderer& r;

		SDL::Colour border_colour;

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

#ifdef DEBUG_GUI_CONTAINERS
			IRenderable::SetParentShape(parent);
#endif
		}

		void RenderGUI()
		{
			r.SetDrawColour(border_colour);
			r.DrawRectF(_shape);
		}

		inline BorderedRect(int render_order, SDL::Renderer& r, const GUIRect& shape, SDL::Colour colour)
			: r(r), border_colour(colour), IRenderable(shape, render_order) {}

	private:
		SDL::FRect _shape;
	};

	struct BorderedFilledRect : public IRenderable
	{
		SDL::Renderer& r;

		SDL::Colour fill_colour;
		SDL::Colour border_colour;

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

#ifdef DEBUG_GUI_CONTAINERS
			IRenderable::SetParentShape(parent);
#endif
		}

		void RenderGUI()
		{
			r.SetDrawColour(fill_colour);
			r.FillRectF(_shape);
			r.SetDrawColour(border_colour);
			r.DrawRectF(_shape);
		}

		inline BorderedFilledRect(int render_order, SDL::Renderer& r, const GUIRect& shape, SDL::Colour fill_colour, SDL::Colour border_colour)
			: r(r), fill_colour(fill_colour), border_colour(border_colour), IRenderable(shape, render_order) {}

	private:
		SDL::FRect _shape;
	};

	struct ContainerLimiter : public IContainer
	{
		SDL::FPoint min_size;
		SDL::FPoint grow_direction { 0.f, 0.f };

		std::vector<std::shared_ptr<IContainer>> containers;

		virtual void SetParentShape(const SDL::FRect& parent)
		{
			SDL::FRect _shape = shape.Get(parent);
			if (parent.w < min_size.w)
			{
				float diff = min_size.w - _shape.w;

				_shape.w = min_size.w;
				_shape.pos.x += (grow_direction.x - 1.f) * .5 * diff;
			}

			for (auto& c : containers)
			{
				assert(c != nullptr);
				c->SetParentShape(_shape);
			}

#ifdef DEBUG_GUI_CONTAINERS
			IContainer::SetParentShape(parent);
#endif
		}
	};

	struct FloatSlider : public IRenderable
	{
		SDL::Renderer r;

		GUIPosition min_position;
		GUIPosition max_position;
		GUIPosition cur_position;

		GUIRect handle_shape;

		float min_value;
		float max_value;
		float cur_value;

		// If the user clicks anywhere in the scrollbar that is not on the knob,
		// the knob will warp to the cursor instead of ignoring the input.
		bool click_warp;

		bool _AddChild(std::shared_ptr<IContainer> child)
		{
			if (handle_container != nullptr) return false;
			handle_container = child;
			return true;
		}

		void _RemoveChild(size_t index)
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			handle_container->parent = nullptr;
			handle_container = nullptr;
		}

		void _ClearChildren()
		{
			if (handle_container == nullptr) return;

			handle_container->parent = nullptr;
			handle_container = nullptr;
		}
		std::shared_ptr<IContainer> _GetChild(size_t index) const
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			return handle_container;
		}

		size_t NumChildren() const
		{
			return handle_container == nullptr ? 0 : 1;
		}

		size_t ChildPosition(std::shared_ptr<IContainer> child) const
		{
			return handle_container != nullptr && child == handle_container ? 0 : ~(size_t)0;
		}

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

			_min_position = min_position.Get(_shape);
			_max_position = max_position.Get(_shape);
			_cur_position = cur_position.Get(_shape);

			_handle_shape = handle_shape.Get(SDL::FRect({ 0.f,0.f }, _shape.size));

			_slider_area = SDL::FRect
			(
				_handle_shape.pos + _min_position,
				_handle_shape.size + (_max_position - _min_position)
			);

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));

#ifdef DEBUG_GUI_CONTAINERS
			IRenderable::SetParentShape(parent);
#endif
		}

		void RenderGUI()
		{
#ifdef DEBUG_GUI_RENDER
			if (click_warp)
			{
				r.SetDrawColour(SDL::YELLOW);
				r.DrawRectF(_slider_area);
			}

			r.SetDrawColour(SDL::GREEN);
			r.DrawRectF(_handle_shape + _cur_position);
			r.SetDrawColour(SDL::RED);
			r.DrawLineF(_min_position, _max_position);

			if (clicker.is_clicked)
			{
				r.SetDrawColour(SDL::WHITE);
				r.DrawPointF(_handle_shape.normToPoint(clicker.click_relative) + _cur_position);
			}
#endif
		}

#ifdef DEBUG_GUI_CONTAINERS
		void RenderAnchors(SDL::Renderer& r) const
		{
			IRenderable::RenderAnchors(r);
			if (handle_container != nullptr) handle_container->RenderAnchors(r);
		}

		void RenderShape(SDL::Renderer& r) const
		{
			IRenderable::RenderShape(r);
			if (handle_container != nullptr) handle_container->RenderShape(r);
		}

		void RenderParent(SDL::Renderer& r) const
		{
			IRenderable::RenderParent(r);
			if (handle_container != nullptr) handle_container->RenderParent(r);
		}
#endif

		FloatSlider(SDL::Renderer& r, const GUIRect& shape, const GUIPosition& min_pos, const GUIPosition& max_pos, const GUIRect& handle_shape, float min_val, float max_val, float init_val, SDL::Button button, bool click_warp = true, int render_order = 0, bool render_enabled = true)
			: r(r),
			IRenderable(shape, render_order, render_enabled),
			min_position(min_pos),
			max_position(max_pos),
			cur_position(MapRange(init_val, min_val, max_val, min_pos, max_pos)),
			handle_shape(handle_shape),
			min_value(min_val),
			max_value(max_val),
			cur_value(init_val),
			click_warp(click_warp),
			clicker(*this, button),
			dragger(*this)
		{}

		~FloatSlider()
		{
			ClearChildren();
		}

		inline constexpr double GetValueNorm() const { return (cur_value - min_value) / (max_value - min_value); }

	private:
		std::shared_ptr<IContainer> handle_container = nullptr;

		SDL::FRect _shape;

		SDL::FPoint _min_position;
		SDL::FPoint _max_position;
		SDL::FPoint _cur_position;
		SDL::FRect _handle_shape;
		SDL::FRect _slider_area;

		void SetFromNorm(double t)
		{
			cur_position = Lerp(t, min_position, max_position);
			cur_value = Lerp(t, min_value, max_value);

			_cur_position = cur_position.Get(_shape);

			_handle_shape = handle_shape.Get(SDL::FRect({ 0.f,0.f }, _shape.size));

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));
		}

		void SetFromPosition(const SDL::FPoint& point)
		{
			SetFromNorm(InverseLerpClamped(point - _handle_shape.normToPoint(clicker.click_relative), _min_position, _max_position));
		}

		struct Clicker : SDL::IInputObserver
		{
			bool is_clicked = false;

			SDL::FPoint click_relative;
			SDL::Button button;

			FloatSlider& slider;

			Clicker(FloatSlider& slider, SDL::Button button)
				: slider(slider),
				button(button)
			{
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEBUTTONDOWN, *this);
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			~Clicker()
			{
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEBUTTONDOWN, *this);
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			void AddSubject(SDL::IInputSubject& s) { return; }
			void RemoveSubject(SDL::IInputSubject& s) { return; }

			void Notify(const SDL::Event& e)
			{
				if (e.button.button != (Uint8)button) return;

				if (e.button.state == SDL_RELEASED)
				{
					if (is_clicked)
					{
						is_clicked = false;
						slider.dragger.OnRelease();
					}
				}
				else
				{
					const SDL::Point click { e.button.x, e.button.y };

					is_clicked = (slider._handle_shape + slider._cur_position).contains(click);

					if (is_clicked)
					{
						click_relative = (slider._handle_shape + slider._cur_position).pointToNorm(click);

						slider.dragger.OnClick();
					}
					else if (slider.click_warp)
					{
						is_clicked = slider._slider_area.contains(click);

						if (!is_clicked) return;

						const double t = InverseLerpClamped(click, slider._min_position, slider._max_position);

						slider.SetFromNorm(t);

						click_relative = (slider._handle_shape + slider._cur_position).pointToNorm(click);

						slider.dragger.OnClick();
					}
				}
			}

		} clicker;

		struct Dragger : SDL::InputObserver
		{
			FloatSlider& slider;

			void OnClick()
			{
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEMOTION, *this);
			}
			void OnRelease()
			{
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEMOTION, *this);
			}

			void Notify(const SDL::Event& e)
			{
				slider.SetFromPosition(SDL::FPoint(e.motion.x, e.motion.y));
			}

			Dragger(FloatSlider& slider) : slider(slider) {}
		} dragger;
	};

	struct IntSlider : public IRenderable
	{
		SDL::Renderer r;

		GUIPosition min_position;
		GUIPosition max_position;
		GUIPosition cur_position;

		GUIRect handle_shape;

		int min_value;
		int max_value;
		int cur_value;

		bool click_warp;

		bool _AddChild(std::shared_ptr<IContainer> child)
		{
			if (handle_container != nullptr) return false;
			handle_container = child;
			return true;
		}

		void _RemoveChild(size_t index)
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			handle_container->parent = nullptr;
			handle_container = nullptr;
		}

		void _ClearChildren()
		{
			if (handle_container == nullptr) return;

			handle_container->parent = nullptr;
			handle_container = nullptr;
		}
		std::shared_ptr<IContainer> _GetChild(size_t index) const
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			return handle_container;
		}

		size_t NumChildren() const
		{
			return handle_container == nullptr ? 0 : 1;

		}
		size_t ChildPosition(std::shared_ptr<IContainer> child) const
		{
			return handle_container != nullptr && child == handle_container ? 0 : ~(size_t)0;
		}

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

			_min_position = min_position.Get(_shape);
			_max_position = max_position.Get(_shape);
			_cur_position = cur_position.Get(_shape);

			_handle_shape = handle_shape.Get(SDL::FRect({ 0.f,0.f }, _shape.size));

			_slider_area = SDL::FRect
			(
				_handle_shape.pos + _min_position,
				_handle_shape.size + (_max_position - _min_position)
			);

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));
		}

		void RenderGUI()
		{
#ifdef DEBUG_GUI_RENDER
			if (click_warp)
			{
				r.SetDrawColour(SDL::YELLOW);
				r.DrawRectF(_slider_area);
			}

			r.SetDrawColour(SDL::GREEN);
			r.DrawRectF(_handle_shape + _cur_position);
			r.SetDrawColour(SDL::RED);
			r.DrawLineF(_min_position, _max_position);

			if (clicker.is_clicked)
			{
				r.SetDrawColour(SDL::WHITE);
				r.DrawPointF(_handle_shape.normToPoint(clicker.click_relative) + _cur_position);
			}
#endif
		}

		IntSlider(SDL::Renderer& r, const GUIRect& shape, const GUIPosition& min_pos, const GUIPosition& max_pos, const GUIRect& handle_shape, std::shared_ptr<IContainer> handle, int min_val, int max_val, int init_val, SDL::Button button, bool click_warp = true, int render_order = 0, bool render_enable = true)
			: r(r),
			IRenderable(shape, render_order, render_enable),
			min_position(min_pos),
			max_position(max_pos),
			cur_position(MapRange(init_val, min_val, max_val, min_pos, max_pos)),
			handle_shape(handle_shape),
			min_value(min_val),
			max_value(max_val),
			cur_value(init_val),
			click_warp(click_warp),
			clicker(*this, button),
			dragger(*this)
		{}

		~IntSlider()
		{
			ClearChildren();
		}

		inline constexpr double GetValueNorm() const { return (cur_value - min_value) / (max_value - min_value); }

	private:
		std::shared_ptr<IContainer> handle_container = nullptr;

		SDL::FRect _shape;

		SDL::FPoint _min_position;
		SDL::FPoint _max_position;
		SDL::FPoint _cur_position;
		SDL::FRect _handle_shape;
		SDL::FRect _slider_area;

		void SetFromNorm(double t)
		{
			cur_position = Lerp(t, min_position, max_position);
			cur_value = Lerp(t, min_value, max_value);

			_cur_position = cur_position.Get(_shape);

			_handle_shape = handle_shape.Get(SDL::FRect({ 0.f,0.f }, _shape.size));

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));
		}

		void SetFromPosition(const SDL::FPoint& point)
		{
			SetFromNorm(InverseLerpClamped(point - _handle_shape.normToPoint(clicker.click_relative), _min_position, _max_position));
		}

		struct Clicker : SDL::IInputObserver
		{
			bool is_clicked = false;

			SDL::FPoint click_relative;
			SDL::Button button;

			IntSlider& slider;

			Clicker(IntSlider& slider, SDL::Button button)
				: slider(slider),
				button(button)
			{
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEBUTTONDOWN, *this);
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			~Clicker()
			{
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEBUTTONDOWN, *this);
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			void AddSubject(SDL::IInputSubject& s) { return; }
			void RemoveSubject(SDL::IInputSubject& s) { return; }

			void Notify(const SDL::Event& e)
			{
				if (e.button.button != (Uint8)button) return;

				if (e.button.state == SDL_RELEASED)
				{
					if (is_clicked)
					{
						is_clicked = false;
						slider.dragger.OnRelease();
					}
				}
				else
				{
					const SDL::Point click { e.button.x, e.button.y };

					is_clicked = (slider._handle_shape + slider._cur_position).contains(click);

					if (is_clicked)
					{
						click_relative = (slider._handle_shape + slider._cur_position).pointToNorm(click);

						slider.dragger.OnClick();
					}
					else if (slider.click_warp)
					{
						is_clicked = slider._slider_area.contains(click);

						if (!is_clicked) return;

						const double t = InverseLerpClamped(click, slider._min_position, slider._max_position);

						slider.SetFromNorm(t);

						click_relative = (slider._handle_shape + slider._cur_position).pointToNorm(click);

						slider.dragger.OnClick();
					}
				}
			}

		} clicker;

		struct Dragger : SDL::InputObserver
		{
			IntSlider& slider;

			void OnClick()
			{
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEMOTION, *this);
			}
			void OnRelease()
			{
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEMOTION, *this);

				slider.cur_position = MapRange
				(
					slider.cur_value,
					slider.min_value,
					slider.max_value,
					slider.min_position,
					slider.max_position
				);

				slider._cur_position = slider.cur_position.Get(slider._shape);

				if (slider.handle_container != nullptr) slider.handle_container->SetParentShape(SDL::FRect(slider._cur_position, slider._shape.size));
			}

			void Notify(const SDL::Event& e)
			{
				slider.SetFromPosition(SDL::FPoint(e.motion.x, e.motion.y));
			}

			Dragger(IntSlider& slider) : slider(slider) {}
		} dragger;
	};

	struct Toggle : public IUpdateable, public IRenderable
	{
		SDL::Renderer r;

		GUIPosition off_position;
		GUIPosition on_position;
		GUIPosition cur_position;

		GUIRect click_area;

		Uint64 scroll_time;

		bool state = false;

		bool _AddChild(std::shared_ptr<IContainer> child)
		{
			if (handle_container != nullptr) return false;
			handle_container = child;
			return true;
		}

		void _RemoveChild(size_t index)
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			handle_container->parent = nullptr;
			handle_container = nullptr;
		}

		void _ClearChildren()
		{
			if (handle_container == nullptr) return;

			handle_container->parent = nullptr;
			handle_container = nullptr;
		}
		std::shared_ptr<IContainer> _GetChild(size_t index) const
		{
			assert(index == 0);
			assert(handle_container != nullptr);
			return handle_container;
		}

		size_t NumChildren() const
		{
			return handle_container == nullptr ? 0 : 1;

		}
		size_t ChildPosition(std::shared_ptr<IContainer> child) const
		{
			return handle_container != nullptr && child == handle_container ? 0 : ~(size_t)0;
		}

		inline Toggle(SDL::Renderer& r, const GUIRect& shape, const GUIPosition& off_pos, const GUIPosition& on_pos, const GUIRect& click_area, bool state, Uint64 scroll_time, SDL::Button button, int render_order = 0, bool render_enabled = true) :
			r(r),
			IRenderable(shape, render_order, render_enabled),
			off_position(off_pos),
			on_position(on_pos),
			cur_position(state ? on_pos : off_pos),
			click_area(click_area),
			state(state),
			scroll_time(scroll_time),
			_t(state ? scroll_time : 0), IUpdateable(),
			clicker(*this, button) {}

		~Toggle()
		{
			ClearChildren();
		}

		void SetParentShape(const SDL::FRect& parent)
		{
			_shape = shape.Get(parent);

			_off_position = off_position.Get(_shape);
			_on_position = on_position.Get(_shape);
			_cur_position = cur_position.Get(_shape);

			_click_area = click_area.Get(_shape);

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));
		}

		void Update(Uint64 dT)
		{
			if (scroll_time == 0)
			{
				_t = 0;

				if (state)
				{
					 cur_position =  on_position;
					_cur_position = _on_position;
				}
				else
				{
					 cur_position =  off_position;
					_cur_position = _off_position;
				}
			}
			else
			{
				if (state)
				{
					_t = scroll_time - _t > dT ? _t + dT : scroll_time;
				}
				else
				{
					_t = _t > dT ? _t - dT : 0;
				}

				cur_position  = LerpClamped(_t / (double)scroll_time,  off_position,  on_position);
				_cur_position = LerpClamped(_t / (double)scroll_time, _off_position, _on_position);
			}

			if (handle_container != nullptr) handle_container->SetParentShape(SDL::FRect(_cur_position, _shape.size));
		}

		void RenderGUI()
		{
#ifdef DEBUG_GUI_RENDER
			r.SetDrawColour(SDL::YELLOW);
			r.DrawRectF(_click_area);
			r.SetDrawColour(SDL::RED);
			r.DrawLineF(_off_position, _on_position);
#endif
		}

	private:
		std::shared_ptr<IContainer> handle_container = nullptr;

		SDL::FRect _shape;

		SDL::FPoint _off_position;
		SDL::FPoint _on_position;
		SDL::FPoint _cur_position;
		SDL::FRect _handle_shape;

		SDL::FRect _click_area;

		Uint64 _t = 0;

		struct Clicker : SDL::IInputObserver
		{
			SDL::Button button;

			Toggle& toggle;

			Clicker(Toggle& toggle, SDL::Button button)
				: toggle(toggle),
				button(button)
			{
				SDL::Input::RegisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			~Clicker()
			{
				SDL::Input::UnregisterEventType(SDL::Event::Type::MOUSEBUTTONUP, *this);
			}

			void AddSubject(SDL::IInputSubject& s) { return; }
			void RemoveSubject(SDL::IInputSubject& s) { return; }

			void Notify(const SDL::Event& e)
			{
				if (e.button.button != (Uint8)button) return;

				const SDL::Point click { e.button.x, e.button.y };

				if (!toggle._click_area.contains(click)) return;

				toggle.state = !toggle.state;
			}

		} clicker;
	};

	struct ContainerGroup : public IContainer
	{
	protected:
		std::vector<std::shared_ptr<IContainer>> _children = {};

		bool _AddChild(std::shared_ptr<IContainer> child) { _children.push_back(child); return true; }
		void _RemoveChild(size_t index) { assert(index < _children.size()); _children.erase(std::next(_children.begin(), index)); }
		void _ClearChildren()
		{
			for (auto& c : _children)
			{
				c->parent = nullptr;
			}

			_children.clear();
		}
		std::shared_ptr<IContainer> _GetChild(size_t index) const
		{
			assert(index < _children.size());
			return _children[index];
		}

	public:
		size_t NumChildren() const { return _children.size(); }
		size_t ChildPosition(std::shared_ptr<IContainer> child) const
		{
			auto it = std::find(_children.begin(), _children.end(), child);
			return it == _children.end() ? ~(size_t)0 : it - _children.begin();
		}

		inline IContainer& Contents() { return *this; }

		inline ContainerGroup(const GUIRect& shape) : IContainer(shape) {}

		inline ~ContainerGroup()
		{
			ClearChildren();
		}

		virtual void SetParentShape(const SDL::FRect& parent)
		{
#ifdef DEBUG_GUI_CONTAINERS
			IContainer::SetParentShape(parent);
#endif
			const SDL::FRect _shape = shape.Get(parent);

			for (auto& c : _children)
			{
				c->SetParentShape(_shape);
			}
		}
	};
}