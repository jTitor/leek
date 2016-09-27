#include "RedBlackTreeBase.h"

using namespace LeEK;

void RedBlackTreeBase::insertFixup(RBTNodeBase* node) {
	RBTNodeBase* cur = node;
	RBTNodeBase* p = cur->Parent();
	while (p->Red()) {
		RBTNodeBase* pp = p->Parent();
		L_ASSERT(pp != &head);
		Side s = (Side)p->ParentSide();
		Side o = (Side)(1 - s); 
		RBTNodeBase* pp_right = pp->Child(o);
		if (pp_right->Red()) {
			p->MakeBlack();
			pp_right->MakeBlack();
			pp->MakeRed();
			cur = pp;
			p = cur->Parent();
		} else {
			if (cur == p->Child(o)) {
				cur = p;
				cur->Rotate(s);
				p = cur->Parent();
			}
			p->MakeBlack();
			pp->MakeRed();
			pp->Rotate(o);
		} 
	}
	head.Child(LEFT)->MakeBlack();
}

void RedBlackTreeBase::eraseFixup(RBTNodeBase* node) {
	RBTNodeBase* cur = node;
	while (!cur->Red() && cur != head.Child(LEFT)) {
		RBTNodeBase* p = cur->Parent();
		Side s = (Side)cur->ParentSide();
		Side o = (Side)(1 - s);
		RBTNodeBase* w = p->Child(o);
		L_ASSERT(w != &head);
		if (w->Red()) {
			L_ASSERT(w->Child(LEFT)->Black() && w->Child(RIGHT)->Black());
			w->MakeBlack();
			p->MakeRed();
			w = w->Child(s);
			p->Rotate(s);
		}
		L_ASSERT(w != &head);
		if (w->Child(LEFT)->Black() && w->Child(RIGHT)->Black()) { 
			w->MakeRed();
			cur = p;
		} else {
			if (w->Child(o)->Black()) {
				w->Child(s)->MakeBlack();
				w->MakeRed();
				RBTNodeBase* c = w->Child(s);
				w->Rotate(o);
				w = c;
				L_ASSERT(w != &head);
			}
			L_ASSERT(w->Child(o)->Red());
			w->MakeRedBlack(p->RedBlack());
			p->MakeBlack();
			w->Child(o)->MakeBlack();
			p->Rotate(s);
			cur = head.Child(LEFT);
		}
	}
	cur->MakeBlack();
}

#ifdef DEBUG_RBTREE
unsigned RedBlackTreeBase::checkHeight(RBTNodeBase* node) const {
	if (node == &head)
		return 0;
	if (node->Black())
		return checkHeight(node->Child(LEFT)) + checkHeight(node->Child(RIGHT)) + 1;
	L_ASSERT(node->Child(LEFT)->Black() && node->Child(RIGHT)->Black());
	unsigned lh = checkHeight(node->Child(LEFT));
	unsigned rh = checkHeight(node->Child(RIGHT));
	L_ASSERT(lh == rh);
	return lh;
}

void RedBlackTreeBase::Check() const {
	L_ASSERT(head.Black());
	L_ASSERT(head.Child(RIGHT) == &head);
	L_ASSERT(head.Child(LEFT) == &head || head.Child(LEFT)->Black());
	checkHeight(head.Child(LEFT));
}
#endif
