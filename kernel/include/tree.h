#ifndef TREE_H
#define TREE_H

#include <util.h>

template <typename TVal>
class Tree;

template <typename TVal>
class TNode {
	void rotate();
public:
	friend class Tree<TVal>;
	TNode(TVal *x): val(x){};
	~TNode() { delete val; }
	TNode<TVal>* f;
	TNode<TVal>* child[2];
	TVal *val;
	void ListNode();
};

template <typename TVal>
class Tree {
	friend class ProcessController;
	TNode<TVal>* root;
	void splay(TNode<TVal>* t, TNode<TVal>* p);
	void deleteNode(TNode<TVal> *node);
public:
	template <typename TFunc>
	TNode<TVal>* find(TFunc check);
	template <typename TFunc>
	void insert(TNode<TVal>* node, TFunc check);
	void ListTree();
	~Tree();
};


template <typename TVal>
void TNode<TVal>::rotate() {
	TNode<TVal> *f = this->f;
	if (f == nullptr) return;
	TNode<TVal> *ff = f->f;
	bool k = (this == f->child[1]);
	bool kk = (f == ff->child[1]);

	f->child[k] = this->child[k^1];
	if (this->child[k^1] != nullptr) this->child[k^1]->f = f;
	this->f = ff;
	if (ff != nullptr) ff->child[kk] = this;
	this->child[k^1] = f;
	f->f = this;
}

template <typename TVal>
void Tree< TVal >::splay(TNode< TVal >* t, TNode< TVal >* p)
{
	while (t -> f != p) {
		TNode<TVal> *f = t->f;
		TNode<TVal> *ff = f->f;
		if (ff != nullptr && ((ff->child[1] == f) ^ (f->child[1] == t))) {
			f -> rotate();
		}
		t->rotate();
	}
}

template <typename TVal>
template <typename TFunc>
TNode<TVal> * Tree<TVal>::find(TFunc check)
{
	TNode<TVal> *pt = root;
	while (pt != nullptr) {
		switch (check(pt->val)){
			case 0:
				pt = pt->child[0];
				break;
			case 1:
				return pt;
				break;
			case 2:
				pt = pt->child[1];
		}
	}
	return nullptr;
}

template<typename TVal>
template <typename TFunc>
void Tree<TVal>::insert(TNode<TVal>* node, TFunc check)
{
	TNode<TVal> *pt = root, *f = nullptr;
	bool k;
	while (pt != nullptr) {
		f = pt;
		if (check(pt->val)) {
			k = 1;
			pt = pt->child[1];
		}
		else {
			k = 0;
			pt = pt->child[0];
		}
	}
	if (f == nullptr) root = node;
	else f->child[k] = node;
}

template<typename TVal>
void TNode<TVal>::ListNode() {
	uPut << &val << '\n';
	if (child[0] != nullptr) child[0]->ListNode();
	if (child[1] != nullptr) child[1]->ListNode();
}

template<typename TVal>
void Tree<TVal>::ListTree() {
	root->ListNode();
}

template<typename TVal>
void Tree<TVal>::deleteNode(TNode<TVal>* node) {
	if (node->child[0] != nullptr) deleteNode(node->child[0]);
	if (node->child[1] != nullptr) deleteNode(node->child[1]);
	delete node;
}

template<typename TVal>
Tree<TVal>::~Tree() {
	deleteNode(root);
}

#endif
