template <typename T>
struct Groupable
{
protected:
	virtual bool _AddChild(std::shared_ptr<Groupable<T>> child) { return false; }
	virtual void _RemoveChild(size_t index) { assert(false); }
	virtual void _ClearChildren() {};
	virtual std::shared_ptr<Groupable<T>> _GetChild(size_t index) const { assert(false); return nullptr; }

public:
	Groupable<T>* parent = nullptr;

	virtual size_t NumChildren() const { return 0; }
	virtual size_t ChildPosition(std::shared_ptr<Groupable<T>> child) const { return ~(size_t)0; }

	inline virtual T& Contents() = 0;

	inline bool AddChild(std::shared_ptr<Groupable<T>> child)
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

	inline std::shared_ptr<Groupable<T>> GetChild(size_t index) const
	{
		assert(index < NumChildren());
		std::shared_ptr<Groupable<T>> child = _GetChild(index);
		assert(child.get() != this);
		assert(ChildPosition(child) == index);
		assert(child->parent == this);
		return child;
	}

	inline void RemoveChild(std::shared_ptr<Groupable<T>> child)
	{
		assert(child->parent == this);
		assert(ChildPosition(child) != ~(size_t)0);
		_RemoveChild(ChildPosition(child));
		child->parent = nullptr;
		assert(ChildPosition(child) == ~(size_t)0);
	}

	inline void RemoveChild(size_t index)
	{
		std::shared_ptr<Groupable<T>> child = GetChild(index);

		_RemoveChild(index);
		child->parent = nullptr;
		assert(ChildPosition(child) == ~(size_t)0);
	}

	inline void ClearChildren()
	{
		_ClearChildren();
		assert(NumChildren() == 0);
	}

	inline static void DeleteTree(Groupable<T>* root)
	{
		if (root == nullptr) return;

		size_t num = root->NumChildren();

		while (num)
		{
			std::shared_ptr<Groupable<T>> child = root->GetChild(--num);

			root->RemoveChild(num);
			assert(root->NumChildren() == num);

			DeleteTree(child.get());
		}
	}

	inline ~Groupable()
	{
		assert(parent == nullptr);
		assert(NumChildren() == 0);
	}
};