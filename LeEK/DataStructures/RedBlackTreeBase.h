#pragma once
#include "DebugUtils/Assertions.h"

#ifndef NULL
#define NULL 0
#endif

namespace LeEK
{
	//weird structs providing type information on a PtrBits instance.
	template<class T> struct PtrBitsTraits {
		typedef T& reference;
	};
	template<> struct PtrBitsTraits<void> {
		typedef void reference;
	};

	//Handles manipulation of bits within the LSBits of a pointer.
	//NumBits is the number of bits of information we want to store in the pointer;
	//naturally, this only works if the pointer is aligned along a large enough boundary.
	//This class doesn't really ensure that the pointer is properly aligned,
	//if it isn't, you'll hit an assert!!!
	/*
		since the smallest values we can directly manipulate are bytes,
		we have to resort to bitmasks, bit shifting, 
		and & and | to set individual bits.
		We can set a bit to 1 by OR'ing against	(1 << bitPosition),
		and we can clear a bit to 0 by AND'ing a field 
		consisting of all other bits BUT that bit set to 0
		~(1 << bitPosition).
		To get the value at a bit position, we AND by (1 << bitPosition);
		it'll return 1 only when the bit is set to 1
	*/
	template<class T, size_t NumBits> class PtrBits {
		//indicates all bits in storage area set to 1
		//by AND'ing with this, you can get just the bits in the pointer
		//by AND'ing with the inverse, you can get the actual address being pointed to
		enum {BITMASK = (1 << NumBits)-1};
		//the actual pointer
		T* mPtr;
	public:
		typedef typename PtrBitsTraits<T>::reference reference;
		PtrBits() : mPtr(0) 
		{
			mPtr = 0;
		}
		PtrBits(T* ptr) : mPtr(ptr)
		{
			//if the ptr ANDed w/ bitmask == 0,
			//then the space taken by the bitmask is manipulable
			//(you can be sure a 1 indicates a 1 bit set, 
			//rather than being a component of the address being pointed to)
			mPtr = ptr;
			L_ASSERT(((size_t)ptr & BITMASK) == 0);
		}
		PtrBits(T* ptr, size_t bits) : mPtr((T*)((size_t)ptr | bits)) {
			L_ASSERT(((size_t)ptr & BITMASK) == 0);
			//also ensure that the bits in storage actually *fit* in the space alloted by NumBits
			L_ASSERT((bits & ~BITMASK) == 0);
		}
		PtrBits(const PtrBits& rhs) : mPtr(rhs) {
			L_ASSERT(((size_t)mPtr & BITMASK) == 0);
		}
		PtrBits& operator=(const T* ptr) {
			L_ASSERT(((size_t)ptr & BITMASK) == 0);
			size_t bits = (size_t)mPtr & BITMASK;
			mPtr = (T*)((size_t)ptr | bits);
			return *this;
		}
		PtrBits& operator=(const PtrBits& rhs) {
			const T* ptr = rhs;
			L_ASSERT(((size_t)ptr & BITMASK) == 0);
			size_t bits = (size_t)mPtr & BITMASK;
			mPtr = (T*)((size_t)ptr | bits);
			return *this;
		}
		size_t getBits() const {
			return (size_t)mPtr & BITMASK;
		}
		//sets bits in the storage space to given value.
		void setBits(size_t bits = BITMASK) {
			L_ASSERT((bits & ~BITMASK) == 0);
			mPtr = (T*)(((size_t)mPtr & ~BITMASK) | bits);
		}
		//sets all bits in storage space to 0.
		void clearBits() {
			mPtr = (T*)((size_t)mPtr & ~BITMASK);
		}
		void swapBits(PtrBits& rhs) {
			size_t bits = getBits();
			set_bits(rhs.getBits());
			rhs.set_bits(bits);
		}
		template<size_t Bit> bool getBit() const {
			size_t bits = (size_t)mPtr & BITMASK;
			//weird, but remember that ! is a logical op
			//and that false is always 0
			//thus !! will give false iff x == 0 and true in all other cases
			return !!(bits & (1 << Bit));
		}
		//sets bit at desired position to 1
		template<size_t Bit> void setBit() {
			mPtr = (T*)((size_t)mPtr | (1 << Bit));
		}
		//sets bit at desired position to 0
		template<size_t Bit> void clearBit() {
			mPtr = (T*)((size_t)mPtr & ~(1 << Bit));
		}
		//all of the following return the address being pointed to
		reference operator*() const {
			L_ASSERT((size_t)mPtr & ~BITMASK);
			return *(T*)((size_t)mPtr & ~BITMASK);
		}
		T* operator->() const {
			L_ASSERT((size_t)mPtr & ~BITMASK);
			return (T*)((size_t)mPtr & ~BITMASK);
		}
		operator T*() const {
			return (T*)((size_t)mPtr & ~BITMASK);
		}
	};

	//the base class for red-black trees.
	class RedBlackTreeBase
	{
	public:
		enum Side {LEFT = 0, RIGHT = 1};
		enum Color {BLACK = 0, RED = 1};

		//nodes of a RBTree.
		/*consists of following:
		* color (red or black)
		* Side relative to parent - is this node the left child or right child?
			* these two can be stored in least sig. bits of the parent pointer
		* parent pointer
		* pointers to left & right children
			* if no children, points to a special node indicating NULL
		* SPECIAL THING FROM HPHA: previous & next pointers, which point to neighboring nodes w/ same key value (key value = size of block)
		* both of these pairs are implemented as 2-entry arrays to simplify rotation algos to one call,
		  rather than two for left and right rotation (GPG7, p.19)
		*regarding the NULL node:
			* is parent of Root node and children of all leaves
			* Root node is LEFT child of NULL node, so that an Iterator will go to the maximum-valued element in tree (GPG7, p.20)
		*/
		class RBTNodeBase
		{
			enum {COLOR_BIT, PARENT_SIDE_BIT, NUM_BIT_TYPES};
			RBTNodeBase* children[2];
			RBTNodeBase* neighbors[2];
			PtrBits<RBTNodeBase,NUM_BIT_TYPES> parent;
		public:
			RBTNodeBase()
			{
				children[LEFT] = this;
				children[RIGHT] = this;
				neighbors[LEFT] = this;
				neighbors[RIGHT] = this;
				parent = this;
			}		
			RBTNodeBase(void* ptr)
			{
				RBTNodeBase* castPtr = (RBTNodeBase*)ptr;
				children[LEFT] = castPtr;
				children[RIGHT] = castPtr;
				neighbors[LEFT] = castPtr;
				neighbors[RIGHT] = castPtr;
				parent = castPtr;
			}
			void DEBUG_FLASH()
			{
				children[LEFT] = this;
				children[RIGHT] = this;
				neighbors[LEFT] = this;
				neighbors[RIGHT] = this;
				parent = this;
			}
			//getter methods
			RBTNodeBase* Parent() const {return parent;}
			RBTNodeBase* Child(Side s) const {return children[s];}
			RBTNodeBase* Left() const {return children[LEFT];}
			RBTNodeBase* Right() const {return children[RIGHT];}
			RBTNodeBase* Neighbour(Side s) const {return neighbors[s];}
			RBTNodeBase* Prev() const {return neighbors[LEFT];}
			RBTNodeBase* Next() const {return neighbors[RIGHT];}
			//are there any neighbors?
			bool Chained() const {return neighbors[LEFT] != this;}
			bool Head() const {return !!parent;}
			Side ParentSide() const {return (Side)(parent.getBits() >> PARENT_SIDE_BIT);}
			void SetParentSide(Side s) {parent.setBits((parent.getBits() & ~(1 << PARENT_SIDE_BIT)) | (s << PARENT_SIDE_BIT));}
			//color manipulation functions
			bool Red() const {return parent.getBit<COLOR_BIT>();}
			bool Black() const {return !parent.getBit<COLOR_BIT>();}
			Color RedBlack() const {return (Color)(parent.getBits() & (1 << COLOR_BIT));}
			void MakeRed() {parent.setBit<COLOR_BIT>();}
			void MakeBlack() {parent.clearBit<COLOR_BIT>();}
			void MakeRedBlack(Color rb) {parent.setBits((parent.getBits() & ~(1 << COLOR_BIT)) | (rb << COLOR_BIT));}
			//is this the null node?
			//check is simple - the null node has the root as left child,
			//so the right child should point back to the null node
			bool IsNullNode() const {return this == children[RIGHT];}
			//rotation function. have fun commenting this
			void Rotate(Side s) {
				L_ASSERT(parent->children[ParentSide()] == this);
				Side o = (Side)(1 - s);
				Side ps = ParentSide();
				RBTNodeBase* top = children[o];
				children[o] = top->children[s];
				children[o]->parent = this;
				children[o]->SetParentSide(o);
				top->parent = parent;   
				top->SetParentSide(ps);
				parent->children[ps] = top;
				top->children[s] = this;
				parent = top;
				SetParentSide(s);
			}
			//can return the predecessor or successor of this node
			//predecessor = node with next smallest value compared to this node
			//successor = node w/ next largest value compared to this node
			//must be properly implemented for iterators on the tree to work
			RBTNodeBase* PredOrSucc(Side s) const {
				RBTNodeBase* cur = const_cast<RBTNodeBase*>(this);
				cur = cur->neighbors[s];
				if (!cur->parent)
					return cur;
				RBTNodeBase* xessor = cur->children[s];
				if (!xessor->IsNullNode()) {
					Side o = (Side)(1 - s);
					while (!xessor->children[o]->IsNullNode())
						xessor = xessor->children[o];
				} else {
					L_ASSERT(!cur->IsNullNode());
					xessor = cur->parent;
					while (cur->ParentSide() == s) {
						L_ASSERT(!xessor->IsNullNode());
						cur = xessor;
						xessor = xessor->parent;
					}
				}
				return xessor;
			}
			RBTNodeBase* Pred() const {return PredOrSucc(LEFT);}
			RBTNodeBase* Succ() const {return PredOrSucc(RIGHT);}
			//can Find the largest or smallest valued node in the tree
			RBTNodeBase* MinOrMax(Side s) const {
				RBTNodeBase* cur = const_cast<RBTNodeBase*>(this);
				RBTNodeBase* minmax = cur;
				while (!cur->IsNullNode()) {
					minmax = cur;
					cur = cur->children[s];
				}
				return minmax;
			}
			RBTNodeBase* Minimum() const {return MinOrMax(LEFT);}
			RBTNodeBase* Maximum() const {return MinOrMax(RIGHT);}
		//protected:
			template<class T> friend class RBTree;

			//attach a node as a child to another node
			void attachTo(RBTNodeBase* parentPram, Side s) {
				neighbors[LEFT] = this;
				neighbors[RIGHT] = this;
				children[LEFT] = parentPram->Child(s);
				children[RIGHT] = parentPram->Child(s);
				parent = parentPram;
				SetParentSide(s);
				parentPram->children[s] = this;
				MakeRed();
			}

			//remove a node, and replace it with a given child
			void substituteWith(RBTNodeBase* child) {
				Side ps = ParentSide();
				child->parent = parent;
				child->SetParentSide(ps);
				parent->children[ps] = child;
			}

			//swap node positions
			void switchWith(RBTNodeBase* node) {
				L_ASSERT(this != node);
				L_ASSERT(node->Head());
				Side nps = node->ParentSide();
				children[LEFT] = node->Child(LEFT);
				children[RIGHT] = node->Child(RIGHT);
				parent = node->parent;
				SetParentSide(nps);
				node->Child(LEFT)->parent = this;
				node->Child(LEFT)->SetParentSide(LEFT);
				node->Child(RIGHT)->parent = this;
				node->Child(RIGHT)->SetParentSide(RIGHT);
				node->parent->children[nps] = this; 
				MakeRedBlack(node->RedBlack());
			}

			void unlink() {
				neighbors[RIGHT]->neighbors[LEFT] = neighbors[LEFT];
				neighbors[LEFT]->neighbors[RIGHT] = neighbors[RIGHT];
			} 
			void link(RBTNodeBase* node) {
				neighbors[LEFT] = node->neighbors[LEFT];
				neighbors[RIGHT] = node;
				neighbors[RIGHT]->neighbors[LEFT] = this;
				neighbors[LEFT]->neighbors[RIGHT] = this;
				children[LEFT] = NULL;
				children[RIGHT] = NULL;
				parent = NULL;
				SetParentSide(LEFT);
				MakeRed();
			}
		};
		RedBlackTreeBase() {}
		RedBlackTreeBase(const RedBlackTreeBase&) {}
		bool Empty() const {return head.Child(LEFT) == &head;}
		#ifdef DEBUG_RBTREE
		void Check() const;
		#endif

	protected:
		RBTNodeBase head;
		void insertFixup(RBTNodeBase* node);
		void eraseFixup(RBTNodeBase* node);
	private:
		#ifdef DEBUG_RBTREE
		unsigned checkHeight(RBTNodeBase* node) const;
		#endif
	};

	//specialized version of the red-black tree. T is the type of the key carried by nodes.
	template<class T> class RedBlackTree : public RedBlackTreeBase {
		RedBlackTree(const RedBlackTree& rhs);
		RedBlackTree& operator=(const RedBlackTree& rhs);
	public:
		class RBTNode : public RBTNodeBase {
		public:
			const T& Data() const {return *(T*)this;}
			T& Data() {return *(T*)this;}
			T* Parent() const {return (T*)RBTNodeBase::Parent();}
			T* Child(Side s) const {return (T*)RBTNodeBase::Child(s);}
			T* Left() const {return (T*)RBTNodeBase::Left();}
			T* Right() const {return (T*)RBTNodeBase::Right();}
			T* Neighbor(Side s) const {return (T*)RBTNodeBase::neighbors[s];}
			T* Next() const {return (T*)RBTNodeBase::Next();}
			T* Prev() const {return (T*)RBTNodeBase::Prev();}
			T* PredOrSucc(Side s) const {return (T*)RBTNodeBase::PredOrSucc(s);}
			T* Pred() const {return (T*)RBTNodeBase::Pred();}
			T* Succ() const {return (T*)RBTNodeBase::Succ();}
			T* MinOrMax(Side s) const {return (T*)RBTNodeBase::MinOrMax(s);}
			T* Minimum() const {return (T*)RBTNodeBase::Minimum();}
			T* Maximum() const {return (T*)RBTNodeBase::Maximum();}
		};

		class ConstIterator;
		class Iterator {
			typedef T& reference;
			typedef T* pointer;
			friend class ConstIterator;
			T* mPtr;
		public:
			Iterator() : mPtr(0) {}
			explicit Iterator(T* ptr) : mPtr(ptr) {}
			reference operator*() const {return mPtr->Data();}
			pointer operator->() const {return &mPtr->Data();}
			operator pointer() const {return &mPtr->Data();}
			Iterator& operator++() {
				mPtr = mPtr->Succ();
				return *this;
			}
			Iterator& operator--() {
				mPtr = mPtr->Pred();
				return *this;
			}
			bool operator==(const Iterator& rhs) const {return mPtr == rhs.mPtr;}
			bool operator!=(const Iterator& rhs) const {return mPtr != rhs.mPtr;}
			T* ptr() const {return mPtr;}
		};

		class ConstIterator {
			typedef const T& reference;
			typedef const T* pointer;
			const T* mPtr;
		public:
			ConstIterator() : mPtr(0) {}
			explicit ConstIterator(const T* ptr) : mPtr(ptr) {}
			ConstIterator(const Iterator& it) : mPtr(it.mPtr) {}
			reference operator*() const {return mPtr->Data();}
			pointer operator->() const {return &mPtr->Data();}
			operator pointer() const {return &mPtr->Data();}
			ConstIterator& operator++() {
				mPtr = mPtr->Succ();
				return *this;
			}
			ConstIterator& operator--() {
				mPtr = mPtr->Pred();
				return *this;
			}
			bool operator==(const ConstIterator& rhs) const {return mPtr == rhs.mPtr;}
			bool operator!=(const ConstIterator& rhs) const {return mPtr != rhs.mPtr;}
			const T* ptr() const {return mPtr;}
		};

		template<class K> Iterator LowerBound(const K& key) {return Iterator(doLowerBound(key));}
		template<class K> ConstIterator LowerBound(const K& key) const {return ConstIterator(doLowerBound(key));}
		template<class K> Iterator UpperBound(const K& key) {return Iterator(doUpperBound(key));}
		template<class K> ConstIterator UpperBound(const K& key) const {return ConstIterator(doUpperBound(key));}
		template<class K> Iterator Find(const K& key) { 
			T* found = doLowerBound(key); 
			if (found == getNullNode() || found->Data() > key)
				return End();
			return Iterator(found);
		}
		template<class K> ConstIterator Find(const K& key) const {
			T* found = doLowerBound(key);
			if (found == getNullNode() || found->Data() > key)
				return End();
			return ConstIterator(found);
		}

		ConstIterator Begin() const {return Minimum();}
		Iterator Begin() {return Minimum();}
		ConstIterator End() const {return ConstIterator(getNullNode());}
		Iterator End() {return Iterator(getNullNode());}
		ConstIterator Root() const {return ConstIterator(getRootNode());}
		Iterator Root() {return Iterator(getRootNode());}
		ConstIterator Minimum() const {return ConstIterator(getRootNode()->Minimum());}
		Iterator Minimum() {return Iterator(getRootNode()->Minimum());}
		ConstIterator Maximum() const {return ConstIterator(getRootNode()->Maximum());}
		Iterator Maximum() {return Iterator(getRootNode()->Maximum());}

		RedBlackTree() : RedBlackTreeBase() {}
		~RedBlackTree() {Clear();}
		Iterator Insert(T* node) {
			this->doInsert(node);
			return Iterator(node);
		}
		Iterator Erase(Iterator where) {
			T* node = where.ptr();
			++where;
			this->doErase(node);
			return where;
		}
		void Erase(T* node) {
			this->doErase(node);
		}
		void Clear() {
			for (Iterator it = this->Begin(); it != this->End();)
				it = this->Erase(it);
		}
	protected:
		T* getNullNode() const {return (T*)&head;}
		T* getRootNode() const {return (T*)head.Child(LEFT);}
		template<class K> T* doLowerBound(const K& key) const {
			T* endNode = getNullNode();
			T* bestNode = getNullNode();
			T* curNode = getRootNode();

			while (curNode != endNode) {
				if (curNode->Data() < key)
					curNode = curNode->Child(RIGHT);
				else {
					bestNode = curNode;
					curNode = curNode->Child(LEFT);
				}
			}
			return bestNode;
		}
		template<class K> T* doUpperBound(const K& key) const {
			T* endNode = getNullNode();
			T* bestNode = getNullNode();
			T* curNode = getRootNode();
			while (curNode != endNode) {
				if (curNode->Data() > key) {
					bestNode = curNode;
					curNode = curNode->Child(LEFT);
				} else
					curNode = curNode->Child(RIGHT);
			}
			return bestNode;
		}
		void doInsert(T* node) {
			T* endNode = getNullNode();
			T* lastNode = getNullNode();
			T* curNode = getRootNode();
			Side s = LEFT;
			while (curNode != endNode) {
				lastNode = curNode;
				s = RIGHT;
				if (node->Data() < curNode->Data()) {
					s = LEFT;
				} else if (!(node->Data() > curNode->Data())) {
					node->link(curNode);
					return;
				}
				curNode = curNode->Child(s);
			}
			node->attachTo(lastNode, s);
			insertFixup(node);
			#ifdef DEBUG_RBTREE
			Check();
			#endif
		}
		void doErase(T* node) {
			if (node->Chained()) {
				if (!node->Head()) {
					L_ASSERT(node->Child(LEFT) == NULL);
					L_ASSERT(node->Child(RIGHT) == NULL);
					node->unlink();
					return;
				}
				T* repl = node->Next();
				L_ASSERT(repl);
				L_ASSERT(repl != getNullNode());
				L_ASSERT(!repl->Parent());
				L_ASSERT(repl->Child(LEFT) == NULL);
				L_ASSERT(repl->Child(RIGHT) == NULL);
				repl->switchWith(node);
				node->unlink();
				return;
			}
			T* endNode = getNullNode();
			T* repl = node;
			Side s = LEFT;
			if (node->Child(RIGHT) != endNode) {
				if (node->Child(LEFT) != endNode) {
					repl = node->Child(RIGHT);
					while (repl->Child(LEFT) != endNode)
						repl = repl->Child(LEFT);
				}
				s = RIGHT;
			}
			L_ASSERT(repl->Child((Side)(1-s)) == endNode);
			bool red = repl->Red();
			T* replChild = repl->Child(s);
			repl->substituteWith(replChild);
			if (repl != node)
				repl->switchWith(node);
			if (!red) 
				eraseFixup(replChild);
			#ifdef DEBUG_RBTREE
			Check();
			#endif
		}
	};
}
