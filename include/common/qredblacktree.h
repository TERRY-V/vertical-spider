/********************************************************************************************
**
** Copyright (C) 2010-2014 Terry Niu (Beijing, China)
** Filename:	qredblacktree.h
** Author:	TERRY-V
** Email:	cnbj8607@163.com
** Support:	http://blog.sina.com.cn/terrynotes
** Date:	2013/11/05
**
*********************************************************************************************/

#ifndef __QREDBLACKTREE_H_
#define __QREDBLACKTREE_H_

#include "qglobal.h"

Q_BEGIN_NAMESPACE

// 红黑树最大元素数量
static const int32_t MAX_TREE_SIZE=400000;

// 红黑树结点颜色
enum NODE_COLOR {RED=0, BLACK};

// QRedBlackTree红黑树类结点
template<typename T_KEY, typename T>
struct RBNode
{
	NODE_COLOR color;			// 结点颜色
	T_KEY key;				// 键
	T value;				// 值
	int32_t count;				// 计数(增加与删除结点时修改计数, 当计数为0时做物理删除)
	RBNode* parent;				// 父节点
	RBNode* left;				// 左子结点
	RBNode* right;				// 右子节点

	// 构造函数
	RBNode() {
		color=BLACK;
		count=1;
		parent=left=right=NULL;
	}

	// 构造函数
	RBNode(T_KEY k, T v) {
		RBNode();
		key=k;
		value=v;
	}

	// 析构函数
	~RBNode() {
		color=BLACK;
		parent=left=right=NULL;
	}
};

// QRedBlackTree红黑树类
template< typename T_KEY, typename T, typename RBTreeNode=RBNode<T_KEY, T> >
class QRedBlackTree {
	public:
		// @函数名: 红黑树构造函数
		inline QRedBlackTree()
		{
			node_num=0;
			m_leaf=new(std::nothrow) RBTreeNode;
			Q_ASSERT(m_leaf!=NULL, "QRedBlackTree: m_leaf is null");

			m_leaf->left=m_leaf;
			m_leaf->right=m_leaf;
			m_root=m_leaf;
			m_root->parent=m_leaf;
		}

		// @函数名: 红黑树析构函数
		virtual ~QRedBlackTree()
		{
			Destroy();
			m_root=NULL;

			delete m_leaf;
			m_leaf=NULL;
		}

		// @函数名: 向红黑树插入元素
		// @参数01: 键
		// @参数02: 值
		// @参数03: 覆盖标记
		// @参数04: 计数标记(若此处使用计数标记，则删除时也应该使用计数标记)
		// @返回值: 成功返回0, 重复返回1, 失败返回<0的错误码
		int32_t insert(T_KEY _key, T _value, bool cover_flag=false, bool count_flag=false)
		{
			if(node_num>MAX_TREE_SIZE)
				return -10;

			RBTreeNode* cur_node=m_root;
			RBTreeNode* pre_node=m_leaf;

			// 确定新结点的插入位置
			while(cur_node!=m_leaf) {
				pre_node=cur_node;

				if(cur_node->key==_key) {
					if(cover_flag)
						cur_node->value=_value;
					else if(count_flag)
						++cur_node->count;
					return 1;
				} else if(_key < cur_node->key) {
					cur_node=cur_node->left;
				} else {
					cur_node=cur_node->right;
				}
			}

			RBTreeNode* new_node=new(std::nothrow) RBTreeNode(_key, _value);
			if(new_node==NULL)
				return -1;

			new_node->parent=pre_node;

			// 新插入结点为根结点
			if(pre_node==m_leaf)
				m_root=new_node;
			else if(_key<pre_node->key)
				pre_node->left=new_node;
			else
				pre_node->right=new_node;

			new_node->left=m_leaf;
			new_node->right=m_leaf;
			new_node->color=RED;

			// 调整红黑树使其满足红黑树性质
			InsertFixup(new_node);
			++node_num;

			return 0;
		}

		// @函数名: 从红黑树上删除元素
		// @参数01: 键
		// @参数02: 计数标记(若插入元素时使用了计数标记，则删除时也应该使用计数标记)
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t remove(T_KEY _key, bool count_flag=false)
		{
			RBTreeNode* node=m_root;

			// 查找待删除结点
			while(true) {
				if(node==m_leaf) {
					return 1;
				} else if(_key==node->key) {
					if(count_flag && --node->count > 0)
						return 2;
					break;
				} else if(_key < node->key) {
					node=node->left;
				} else {
					node=node->right;
				}
			}

			RBTreeNode* del_node=NULL;
			RBTreeNode* cur_node=NULL;

			if(node->left==m_leaf || node->right==m_leaf) {
				del_node=node;
			} else {
				del_node=node->right;
				// 查找待删除结点的后继结点
				while(del_node->left!=m_leaf)
					del_node=del_node->left;
			}

			if(del_node->left!=m_leaf)
				cur_node=del_node->left;
			else
				cur_node=del_node->right;

			cur_node->parent=del_node->parent;

			if(del_node->parent==m_leaf)
				m_root=cur_node;
			else if(del_node==del_node->parent->left)
				del_node->parent->left=cur_node;
			else
				del_node->parent->right=cur_node;

			if(del_node!=node) {
				node->key=del_node->key;
				node->value=del_node->value;
			}

			if(del_node->color==BLACK)	
				DeleteFixup(cur_node);

			delete del_node;
			--node_num;

			return 0;
		}

		// @函数名: 红黑树查找元素
		// @参数01: 键
		// @参数02: 值
		// @返回值: 成功返回0, 失败返回<0的错误码
		int32_t search(T_KEY _key, T& _value)
		{
			RBTreeNode* node=m_root;
			while(true) {
				if(node==m_leaf)
					return -1;
				else if(_key==node->key)
					break;
				else if(_key<node->key)
					node=node->left;
				else
					node=node->right;
			}
			_value=node->value;
			return 0;
		}

		// @函数名: 红黑树的高度
		void height()
		{OutputHeight(m_root, 0);}

		// @函数名: 红黑树元素数量
		int32_t size()
		{return node_num;}

	private:
		// @函数名: 递归输出红黑树的高度
		void OutputHeight(RBTreeNode* root, int32_t height)
		{
			if(root==m_leaf) {
				Q_DEBUG("QRedBlackTree: height=(%d)", height);
				return;
			}
			OutputHeight(root->left, height + root->color);
			OutputHeight(root->right, height + root->color);
		}

		// @函数名: 增加元素时调整红黑树使其满足红黑树性质
		int32_t InsertFixup(RBTreeNode* node)
		{
			RBTreeNode* uncle_node=NULL;
			while(node->parent->color==RED) {
				if(node->parent==node->parent->parent->left) {
					uncle_node=node->parent->parent->right;
					// 当前结点的父结点是红色，并且叔叔结点也是红色
					if(uncle_node->color==RED) {
						node->parent->color=BLACK;
						uncle_node->color=BLACK;
						node=uncle_node->parent;
						node->color=RED;
					} else {
						// 当前结点是父结点的右子结点
						if(node==node->parent->right) {
							node=node->parent;
							LeftRotate(node);
						}
						// 当前结点是父结点的左子结点
						node->parent->color=BLACK;
						node->parent->parent->color=RED;
						RightRotate(node->parent->parent);
						break;
					}
				} else {
					uncle_node=node->parent->parent->left;
					if(uncle_node->color==RED) {
						node->parent->color=BLACK;
						uncle_node->color=BLACK;
						node=uncle_node->parent;
						node->color=RED;
					} else {
						if(node==node->parent->left) {
							node=node->parent;
							RightRotate(node);
						}
						node->parent->color=BLACK;
						node->parent->parent->color=RED;
						LeftRotate(node->parent->parent);
						break;
					}
				}
			}
			m_root->color=BLACK;
			return 0;
		}

		// @函数名: 删除元素时调整红黑树使其符合红黑树性质
		int32_t DeleteFixup(RBTreeNode* node)
		{
			RBTreeNode* brother=NULL;
			while(node!=m_root&&node->color==BLACK) {
				if(node==node->parent->left) {
					brother=node->parent->right;
					if(brother->color==RED) {
						brother->color=BLACK;
						node->parent->color=RED;
						LeftRotate(node->parent);
						brother=node->parent->right;
					}
					if(brother->left->color==BLACK&&brother->right->color==BLACK) {
						brother->color=RED;
						node=node->parent;
					} else {
						if(brother->right->color==BLACK) {
							brother->color=RED;
							brother->left->color=BLACK;
							RightRotate(brother);
							brother=node->parent->right;
						}
						brother->color=brother->parent->color;
						brother->parent->color=BLACK;
						brother->right->color=BLACK;
						LeftRotate(brother->parent);
						node=m_root;
					}
				} else {
					brother=node->parent->left;
					if(brother->color==RED) {
						brother->color==BLACK;
						node->parent->color=RED;
						RightRotate(node->parent);
						brother=node->parent->left;
					}
					if(brother->left->color==BLACK&&brother->right->color==BLACK) {
						brother->color=RED;
						node=node->parent;
					} else {
						if(brother->left->color==BLACK) {
							brother->color==RED;
							brother->right->color==BLACK;
							LeftRotate(brother);
							brother=node->parent->left;
						}
						brother->color=brother->parent->color;
						brother->parent->color=BLACK;
						brother->left->color=BLACK;
						RightRotate(brother->parent);
						node=m_root;
					}
				}
			}
			node->color=BLACK;
			return 0;
		}

		// @函数名: 红黑树左旋转
		void LeftRotate(RBTreeNode* sub_root)
		{
			RBTreeNode* temp_node=sub_root->right;

			sub_root->right=temp_node->left;
			if(temp_node->left!=m_leaf)
				temp_node->left->parent=sub_root;

			temp_node->parent=sub_root->parent;

			if(sub_root->parent==m_leaf)
				m_root=temp_node;
			else if(sub_root==sub_root->parent->left)
				sub_root->parent->left=temp_node;
			else
				sub_root->parent->right=temp_node;

			temp_node->left=sub_root;
			sub_root->parent=temp_node;
		}

		// @函数名: 红黑树右旋转
		void RightRotate(RBTreeNode* sub_root)
		{
			RBTreeNode* temp_node=sub_root->left;

			sub_root->left=temp_node->right;
			if(temp_node->right!=m_leaf)
				temp_node->right->parent=sub_root;

			temp_node->parent=sub_root->parent;

			if(sub_root->parent==m_leaf)
				m_root=temp_node;
			else if(sub_root==sub_root->parent->left)
				sub_root->parent->left=temp_node;
			else
				sub_root->parent->right=temp_node;

			temp_node->right=sub_root;
			sub_root->parent=temp_node;
		}

		int32_t GetTreeSize()
		{
			tree_size=0;
			GetTreeSize(m_root);
			return tree_size;
		}

		void GetTreeSize(RBTreeNode* node)
		{
			if(node==m_leaf)
				return;

			++tree_size;

			GetTreeSize(node->left);
			GetTreeSize(node->right);
		}

		// @函数名: 销毁整棵红黑树
		int32_t Destroy()
		{
			if(m_root==m_leaf)	
				return 1;
			assert(Destroy(m_root)==0);
			m_root=m_leaf;
			return 0;
		}

		// @函数名: 递归删除整棵红黑树的所有节点
		int32_t Destroy(RBTreeNode* root)
		{
			if(root->left!=m_leaf)
				Destroy(root->left);
			if(root->right!=m_leaf)
				Destroy(root->right);
			delete root;
			return 0;
		}

	private:
		RBTreeNode* m_root;			// 根结点
		RBTreeNode* m_leaf;			// 叶子结点
		int32_t node_num;			// 当前已使用的节点个数
		int32_t tree_size;			// QRedBlackTree 大小
};

Q_END_NAMESPACE

#endif	// __QREDBLACKTREE_H_
