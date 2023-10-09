#include <SDL.hpp>
#include <algorithm>
#include <map>
#include <assert.h>

namespace GUI
{
	// A position with awareness of the position and size of its parent container.
	// To be used for GUI in a hierarchy.
	struct GUIPosition
	{
		// Normalised position [(0,0),(1,1)] relative to parent.
		SDL::FPoint anchor;
		// Constant offset from evaluated relative position.
		SDL::FPoint offset;

		inline constexpr GUIPosition(const SDL::FPoint& anchor, const SDL::FPoint& offset)
			: anchor(anchor), offset(offset) {}

		// Evaluates this GUIPosition relative to a parent into an FPoint.
		inline constexpr SDL::FPoint Get(const SDL::FRect& parent) const
		{
			return parent.pos + parent.size * anchor + offset;
		}

		inline constexpr GUIPosition operator+(const GUIPosition& other) const
		{
			return GUIPosition(anchor + other.anchor, offset + other.offset);
		}

		inline constexpr GUIPosition operator-(const GUIPosition& other) const
		{
			return GUIPosition(anchor - other.anchor, offset - other.offset);
		}

		inline constexpr GUIPosition operator*(double factor) const
		{
			return { anchor * factor, offset * factor };
		}

		inline constexpr GUIPosition operator/(double factor) const
		{
			return { anchor / factor, offset / factor };
		}

		inline constexpr GUIPosition& operator+=(const GUIPosition& other)
		{
			anchor = anchor + other.anchor;
			offset = offset + other.offset;
			return *this;
		}

		inline constexpr GUIPosition& operator-=(const GUIPosition& other)
		{
			anchor = anchor - other.anchor;
			offset = offset - other.offset;
			return *this;
		}

		inline constexpr GUIPosition& operator*=(double factor)
		{
			anchor = anchor * factor;
			offset = offset * factor;
			return *this;
		}

		inline constexpr GUIPosition& operator/=(double factor)
		{
			anchor = anchor / factor;
			offset = offset / factor;
			return *this;
		}
	};

	// A size with awareness of the size of its parent container.
	// To be used for GUI in a hierarchy.
	struct GUISize
	{
		// Normalised size [(0,0),(1,1)] relative to parent.
		SDL::FPoint anchor;
		// Constant offset from evaluated relative size.
		SDL::FPoint offset;

		inline constexpr GUISize(const SDL::FPoint& anchor, const SDL::FPoint& offset)
			: anchor(anchor), offset(offset) {}

		// Evaluates this GUISize relative to a parent into an FPoint.
		inline const SDL::FPoint Get(const SDL::FRect& parent) const
		{
			return parent.size * anchor + offset;
		}

		inline constexpr GUISize operator+(const GUISize& other) const
		{
			return GUISize(anchor + other.anchor, offset + other.offset);
		}

		inline constexpr GUISize operator-(const GUISize& other) const
		{
			return GUISize(anchor - other.anchor, offset - other.offset);
		}

		inline constexpr GUISize operator*(double factor) const
		{
			return { anchor * factor, offset * factor };
		}

		inline constexpr GUISize operator/(double factor) const
		{
			return { anchor / factor, offset / factor };
		}

		inline constexpr GUISize& operator+=(const GUISize& other)
		{
			anchor = anchor + other.anchor;
			offset = offset + other.offset;
			return *this;
		}

		inline constexpr GUISize& operator-=(const GUISize& other)
		{
			anchor = anchor - other.anchor;
			offset = offset - other.offset;
			return *this;
		}

		inline constexpr GUISize& operator*=(double factor)
		{
			anchor = anchor * factor;
			offset = offset * factor;
			return *this;
		}

		inline constexpr GUISize& operator/=(double factor)
		{
			anchor = anchor / factor;
			offset = offset / factor;
			return *this;
		}
	};

	// A rectangle with awareness of the size and position of its parent container.
	// To be used for GUI in a hierarchy.
	struct GUIRect
	{
		// The upper-left corner of the rectangle, with awareness of its parent container.
		GUIPosition position;
		// The size of the rectangle, with awareness of its parent container.
		GUISize size;

		inline constexpr GUIRect(const GUIPosition& position, const GUISize& size) : position(position.anchor, position.offset), size(size.anchor, size.offset) {}
		inline constexpr GUIRect(
			const SDL::FPoint& position_relative,
			const SDL::FPoint& position_offset,
			const SDL::FPoint& size_relative,
			const SDL::FPoint& size_offset
		) :
			position(position_relative, position_offset),
			size(size_relative, size_offset) {}

		// Evaluates this GUIRect relative to a parent into an FRect.
		inline constexpr SDL::FRect Get(const SDL::FRect& parent) const
		{
			return
			{
				position.Get(parent),
				this->size.Get(parent)
			};
		}

		inline constexpr GUIRect operator+(const GUIPosition& other) const
		{
			return GUIRect(position + other, size);
		}

		inline constexpr GUIRect operator-(const GUIPosition& other) const
		{
			return GUIRect(position - other, size);
		}

		inline constexpr GUIRect operator+(const GUISize& other) const
		{
			return GUIRect(position, size + other);
		}

		inline constexpr GUIRect operator-(const GUISize& other) const
		{
			return GUIRect(position, size - other);
		}

		inline constexpr GUIRect& operator+=(const GUIPosition& other)
		{
			position += other;
			return *this;
		}

		inline constexpr GUIRect& operator-=(const GUIPosition& other)
		{
			position -= other;
			return *this;
		}

		inline constexpr GUIRect& operator+=(const GUISize& other)
		{
			size += other;
			return *this;
		}

		inline constexpr GUIRect& operator-=(const GUISize& other)
		{
			size -= other;
			return *this;
		}
	};

	// A base type for GUI components with awareness of each others' size and positions.
	struct IContainer
	{
	protected:
		virtual bool _AddChild(std::shared_ptr<IContainer> child) { return false; }
		virtual void _RemoveChild(size_t index) { assert(false); }
		virtual void _ClearChildren() {};
		virtual std::shared_ptr<IContainer> _GetChild(size_t index) const { assert(false); return nullptr; }

	public:
		IContainer* parent = nullptr;

		virtual size_t NumChildren() const { return 0; }
		virtual size_t ChildPosition(std::shared_ptr<IContainer> child) const { return ~(size_t)0; }

		inline bool AddChild(std::shared_ptr<IContainer> child)
		{
			if (child == nullptr) return false;

			if (child->parent == this)
			{
				assert(ChildPosition(child) != ~(size_t)0);
				return true;
			}

			if (child->parent != nullptr)
			{
				child->parent->RemoveChild(child);
			}

			assert(ChildPosition(child) == ~(size_t)0);

			if (_AddChild(child))
			{
				assert(ChildPosition(child) != ~(size_t)0);
				child->parent = this;
				return true;
			}
			else
			{
				assert(ChildPosition(child) == ~(size_t)0);
				child->parent = nullptr;
				return false;
			}
		}

		inline std::shared_ptr<IContainer> GetChild(size_t index) const
		{
			assert(index < NumChildren());
			std::shared_ptr<IContainer> child = _GetChild(index);
			assert(child.get() != this);
			assert(ChildPosition(child) == index);
			assert(child->parent == this);
			return child;
		}

		inline void RemoveChild(std::shared_ptr<IContainer> child)
		{
			assert(child->parent == this);
			assert(ChildPosition(child) != ~(size_t)0);
			_RemoveChild(ChildPosition(child));
			child->parent = nullptr;
			assert(ChildPosition(child) == ~(size_t)0);
		}

		inline void RemoveChild(size_t index)
		{
			std::shared_ptr<IContainer> child = GetChild(index);

			_RemoveChild(index);
			child->parent = nullptr;
			assert(ChildPosition(child) == ~(size_t)0);
		}

		inline void ClearChildren()
		{
			_ClearChildren();
			assert(NumChildren() == 0);
		}

		inline static void DeleteTree(IContainer* root)
		{
			if (root == nullptr) return;

			size_t num = root->NumChildren();

			while (num)
			{
				std::shared_ptr<IContainer> child = root->GetChild(--num);

				root->RemoveChild(num);
				assert(root->NumChildren() == num);

				DeleteTree(child.get());
			}
		}

		// The shape of this container relative to its parent.
		GUIRect shape;

#ifndef DEBUG_GUI_CONTAINERS
		
		inline IContainer(const GUIRect& shape                    ) : shape(shape) {}
		inline ~IContainer()
		{
			assert(parent == nullptr);
			assert(NumChildren() == 0);
		}

		// This function is called whenever the parent of this container is reshaped.
		// Use it to recalculate your own shapes and GUI aware types.
		virtual void SetParentShape(const SDL::FRect& parent)
		{
			size_t num = NumChildren();

			const SDL::FRect _shape = shape.Get(parent);

			while (num)
			{
				GetChild(--num)->SetParentShape(_shape);
			}
		}

#else
		SDL::FRect parent_shape;

		inline IContainer(const GUIRect& shape) : shape(shape) { _containers.push_back(this); }
		inline ~IContainer()
		{
			std::remove(_containers.begin(), _containers.end(), this);
			assert(parent == nullptr);
			assert(NumChildren() == 0);
		}

		// This function is called whenever the parent of this container is reshaped.
		// Use it to recalculate your own shapes and GUI aware types.
		virtual void SetParentShape(const SDL::FRect& parent)
		{
			this->parent_shape = parent;

			size_t num = NumChildren();

			const SDL::FRect _shape = shape.Get(parent);

			while (num)
			{
				GetChild(--num)->SetParentShape(_shape);
			}
		}

		// Renders corners of the relative shape within the parent before
		// offsets are applied.
		void RenderAnchors(SDL::Renderer& r) const
		{
			const SDL::FRect _anchor_shape = { parent_shape.pos + parent_shape.size * shape.position.anchor, parent_shape.size * shape.size.anchor };

			const SDL::FPoint top_left = _anchor_shape.topLeft();
			const SDL::FPoint top_right = _anchor_shape.topRight();
			const SDL::FPoint bot_right = _anchor_shape.bottomRight();
			const SDL::FPoint bot_left = _anchor_shape.bottomLeft();

			r.SetDrawColour(SDL::WHITE);
			r.DrawLinesF({ top_left,  top_left + SDL::FPoint(-10.f, -4.f), top_left + SDL::FPoint(-4.f, -10.f), top_left });
			r.DrawLinesF({ top_right, top_right + SDL::FPoint(4.f, -10.f), top_right + SDL::FPoint(10.f, -4.f), top_right });
			r.DrawLinesF({ bot_right, bot_right + SDL::FPoint(10.f,   4.f), bot_right + SDL::FPoint(4.f,  10.f), bot_right });
			r.DrawLinesF({ bot_left,  bot_left + SDL::FPoint(-4.f,  10.f), bot_left + SDL::FPoint(-10.f,   4.f), bot_left });

			r.SetDrawColour(SDL::ORANGE);
			if (_anchor_shape.w == 0)
			{
				if (_anchor_shape.h == 0)
				{
					r.DrawPointF(top_left);
				}
				else
				{
					r.DrawLineF(top_left, bot_right);
				}
			}
			else
			{
				if (_anchor_shape.h == 0)
				{
					r.DrawLineF(top_left, bot_right);
				}
				else
				{
					r.DrawRectF(_anchor_shape);
				}
			}
		}

		// Renders the achors of all containers that exist.
		static void RenderAllAnchors(SDL::Renderer& r)
		{
			for (auto c : _containers)
			{
				c->RenderAnchors(r);
			}
		}

		// Renders outline of shape relative to stored parent in screen coordinates.
		void RenderShape(SDL::Renderer& r) const
		{
			SDL::FRect _shape = shape.Get(parent_shape);

			r.SetDrawColour(SDL::AZURE);

			if (_shape.w == 0)
			{
				if (_shape.h == 0)
				{
					r.DrawPointF(_shape.pos);
				}
				else
				{
					r.DrawLineF(_shape.pos, _shape.pos + _shape.size);
				}
			}
			else
			{
				if (_shape.h == 0)
				{
					r.DrawLineF(_shape.pos, _shape.pos + _shape.size);
				}
				else
				{
					r.DrawRectF(_shape);
				}
			}
		}

		// Renders the shapes of all containers that exist.
		static void RenderAllShapes(SDL::Renderer& r)
		{
			for (auto c : _containers)
			{
				c->RenderShape(r);
			}
		}

		// Renders outline of last parent shape received in screen coordinates.
		void RenderParent(SDL::Renderer& r) const
		{
			r.SetDrawColour(SDL::RED);

			if (parent_shape.w == 0)
			{
				if (parent_shape.h == 0)
				{
					r.DrawPointF(parent_shape.pos);
				}
				else
				{
					r.DrawLineF(parent_shape.pos, parent_shape.pos + parent_shape.size);
				}
			}
			else
			{
				if (parent_shape.h == 0)
				{
					r.DrawLineF(parent_shape.pos, parent_shape.pos + parent_shape.size);
				}
				else
				{
					r.DrawRectF(parent_shape);
				}
			}
		}

		// Renders the parents of all containers that exist.
		static void RenderAllParents(SDL::Renderer& r)
		{
			for (auto c : _containers)
			{
				c->RenderParent(r);
			}
		}

	private:
		inline static std::vector<IContainer*> _containers = {};
#endif // DEBUG_GUI_CONTAINERS

	};

	// A base type for GUI components that may be rendered to the screen.
	struct IRenderable : public IContainer
	{
		inline IRenderable(const GUIRect& shape, int render_order = 0, bool render_enabled = true) : IContainer(shape) {
			_order = render_order;
			_enabled = render_enabled;
			_Add(*this);
		}

		inline constexpr int GetOrder() { return _order; }
		inline void SetOrder(int order)
		{
			_Remove(*this);
			_order = order;
			_Add(*this);
		}

		inline constexpr bool GetEnable() { return _enabled; }
		inline void SetEnable(bool enable)
		{
			if (_enabled == enable) return;
			_enabled = enable;

			if (_enabled) OnEnable();
			else OnDisable();
		}

		virtual void OnEnable() {}
		virtual void OnDisable() {}

		~IRenderable()
		{
			_Remove(*this);
		}

		virtual void RenderGUI() = 0;
		static void RenderAllGUI()
		{
			for (auto& it : _renderables)
			{
				for (auto r : it.second)
				{
					if (!r->_enabled) continue;

					r->RenderGUI();
				}
			}
		}

	private:
		inline static std::map<int,std::vector<IRenderable*>> _renderables = {};
		int _order = 0;
		bool _enabled = true;
		inline static void _Add(IRenderable& r)
		{
			if (_renderables.count(r._order) == 0)
			{
				_renderables[r._order] = std::vector<IRenderable*>();
			}

			_renderables[r._order].push_back(&r);
		}
		inline static void _Remove(IRenderable& r)
		{
			std::vector<IRenderable*>& _v = _renderables[r._order];
			
			std::remove(_v.begin(), _v.end(), &r);
		}
	};

	struct IUpdateable
	{
		IUpdateable()
		{
			_Add(*this);
		}

		~IUpdateable()
		{
			_Remove(*this);
		}

		inline constexpr bool GetEnable() { return _enabled; }
		inline void SetEnable(bool enable)
		{
			if (_enabled == enable) return;
			_enabled = enable;

			if (_enabled) OnEnable();
			else OnDisable();
		}

		virtual void OnEnable() {}
		virtual void OnDisable() {}

		virtual void Update(Uint64 dT) = 0;

		inline static void UpdateAll(Uint64 dT)
		{
			for (auto r : _updateables)
			{
				if (!r->_enabled) continue;
				r->Update(dT);
			}
		}

	private:
		inline static std::vector<IUpdateable*> _updateables = {};
		bool _enabled = true;

		inline static void _Add(IUpdateable& u)
		{
			_updateables.push_back(&u);
		}

		inline static void _Remove(IUpdateable& u)
		{
			std::remove(_updateables.begin(), _updateables.end(), &u);
		}
	};
}