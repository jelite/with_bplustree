#include "btree.h"
#include <typeinfo>
/**
 * @file btree.cpp
 * Implementation of a B-tree class which can be used as a generic dictionary
 * (insert-only). Designed to take advantage of caching to be faster than
 * standard balanced binary search trees.
 *
 * @author Matt Joras
 * @date Winter 2013
 */

using std::vector;
using std::queue;

/**
 * Finds the value associated with a given key.
 * @param key The key to look up.
 * @return The value (if found), the default V if not.
 */
template <class K, class V>
V BTree<K, V>::find(const K& key) const
{
    return root == nullptr ? V() : find(root, key);
}

/**
 * Finds the value associated with a given key.
 * @param key The key to look up.
 * @return The value (if found), the default V if not.
 */
template <class K, class V>
void BTree<K, V>::remove(K& key)
{
  vector<size_t> before_idxes;
    if(root != nullptr)
    {
      remove(root, key, before_idxes);
    }
}
/**
 * Finds the value associated with a given key.
 * @param key The key to look up.

 */
template <class K, class V>
void BTree<K, V>::print()
{
  print(root);
}

/**
 * Private recursive version of the find function.
 * @param subroot A reference of a pointer to the current BTreeNode.
 * @param key The key we are looking up.
 * @return The value (if found), the default V if not.
 */
template <class K, class V>
V BTree<K, V>::find(const BTreeNode* subroot, const K& key) const
{
  size_t first_larger_idx = insertion_idx(subroot->elements, key);

  if (!subroot->elements.empty() && first_larger_idx < subroot->elements.size()) {
    DataPair found = subroot->elements[first_larger_idx];
    if (found.key == key)
    {
      return found.value;
    }
  }

  if (!subroot->is_leaf)
  {
    return find (subroot->children[first_larger_idx], key);
  }
  else
  {
    return V();
  }
}

template <class K, class V>
bool BTree<K,V>::borrow_from_sibilings(BTreeNode* parent, size_t idx, size_t before_idx)
{
  BTreeNode* left_sibiling;
  BTreeNode* right_sibiling;

  if(before_idx != 0)
  {
    left_sibiling = parent->children[before_idx-1];
    if(left_sibiling->elements.size() > (order-1)/2)
    {
      //1. 자식에게 부모의 것 부여. 2. 남는 자식이 부모에게 부여. 3. 남는자식요소 삭제
      parent->children[before_idx]->elements[idx] = parent->elements[before_idx-1];
      parent->elements[before_idx-1] = left_sibiling->elements.back();
      left_sibiling->elements.pop_back();

      return true;
    }
  }
  else if(before_idx != parent->elements.size())
  {
    right_sibiling = parent->children[before_idx+1];
    if(right_sibiling->elements.size() > (order-1)/2)
    {
      parent->children[before_idx]->elements[idx] = parent->elements[before_idx+1];
      parent->elements[before_idx+1] = left_sibiling->elements.back();
      right_sibiling->elements.pop_back();

      return true;
    }
  }
  return false;
}

template <class K, class V>
bool BTree<K,V>::borrow_from_parent(BTreeNode* parent, size_t idx, size_t before_idx)
{
  BTreeNode* left_sibiling;
  BTreeNode* right_sibiling;
  
  //delete parent->children[before_idx];
  std::cout << "delete is .... \n";
  std::cout << parent->children[before_idx]->elements[idx].key << std::endl;

  if(before_idx != 0)
  {
    left_sibiling = parent->children[before_idx-1];
    left_sibiling->elements.push_back(parent->elements[before_idx-1]);
    parent->elements.erase(parent->elements.begin() + (before_idx-1));
    parent->children.erase(parent->children.begin() + (before_idx));  
    return true;
  }
  else
  {
    right_sibiling = parent->children[before_idx+1];
    right_sibiling->elements.insert(right_sibiling->elements.begin(), parent->elements[before_idx]);
    parent->elements.erase(parent->elements.begin() + before_idx);
    parent->children.erase(parent->children.begin() + before_idx);
    return true;
  }
  return false;
}

template <class K, class V>
void BTree<K,V>::remove_and_reconstruct(BTreeNode* parent, size_t idx, std::vector<size_t> before_idxes)
{
  std::cout << "remove_and_reconstruct test...1\n";
  std::cout << parent->children[before_idxes.back()]->elements[idx].key <<std::endl;

  size_t before_idx = before_idxes.back();
  BTreeNode* current_node = parent->children[before_idx];
  BTreeNode* merged_node;

  if(current_node->is_leaf)
  {
    if(before_idx != 0)
    {
      auto cur_begin_iter = current_node->elements.begin();
      auto cur_idx_iter = current_node->elements.begin() + idx;
      auto cur_end_iter = current_node->elements.end();
      auto left_sibiling = parent->children[before_idx-1];

      left_sibiling->elements.push_back(parent->elements[before_idx-1]);
      left_sibiling->elements.insert(left_sibiling->elements.end(), cur_begin_iter, cur_idx_iter);
      left_sibiling->elements.insert(left_sibiling->elements.end(), cur_idx_iter+1, cur_end_iter);

      parent->elements.erase(parent->elements.begin() + before_idx-1);
      parent->children.erase(parent->children.begin() + before_idx);

      merged_node = left_sibiling;
    }
    else
    {
      auto cur_begin_iter = current_node->elements.begin();
      auto cur_idx_iter = current_node->elements.begin() + idx;
      auto cur_end_iter = current_node->elements.end();
      auto right_sibiling = parent->children[before_idx+1];

      right_sibiling->elements.insert(right_sibiling->elements.begin(), parent->elements[before_idx]);
      right_sibiling->elements.insert(right_sibiling->elements.begin(), cur_idx_iter+1, cur_end_iter);
      right_sibiling->elements.insert(right_sibiling->elements.begin(), cur_begin_iter, cur_idx_iter);

      parent->elements.erase(parent->elements.begin() + before_idx-1);
      parent->children.erase(parent->children.begin() + before_idx);    

      merged_node = right_sibiling;
    }
    before_idxes.pop_back();
    before_idx = before_idxes.back();

    std::cout << "remove_and_reconstruct test...2\n";
    for(auto elem : merged_node->elements)
    {
      std::cout << elem.key << " " << std::endl;
    }
    std::cout << "remove_and_reconstruct test...3\n";
    std::cout << merged_node->parent->elements.size() << std::endl;
    std::cout << "remove_and_reconstruct test...4\n";
    std::cout << before_idx << std::endl;
    std::cout << "is true???\n";
    size_t parent_insert_idx;
    if(before_idx != 0)
    {
      parent_insert_idx = before_idx-1;
      merged_node->parent = parent->parent->children[parent_insert_idx];
      merged_node->parent->elements.push_back(parent->parent->elements[before_idx-1]);
      merged_node->parent->children.push_back(merged_node);

      parent->parent->elements.erase(parent->parent->elements.begin()+before_idx-1);
      parent->parent->children.erase(parent->parent->children.begin()+before_idx);
    }
    else
    {
      parent_insert_idx = before_idx+1;
      merged_node->parent = parent->parent->children[parent_insert_idx];
      merged_node->parent->elements.insert(merged_node->parent->elements.begin(), parent->parent->elements[before_idx]);
      merged_node->parent->children.insert(merged_node->parent->children.begin(), merged_node);

      parent->parent->elements.erase(parent->parent->elements.begin()+before_idx);
      parent->parent->children.erase(parent->parent->children.begin()+before_idx);
    }
    if(merged_node->parent->elements.size() >= order) split_child(merged_node->parent->parent, parent_insert_idx);
    if(root->elements.size() == 0)
    {
      root = merged_node->parent;
    }
  }
}

template <class K, class V>
void BTree<K,V>::remove_from_leaf(BTreeNode* subroot, size_t idx, std::vector<size_t> before_idxes)
{
  bool is_borrow_from_other;
  if(subroot->elements.size() > (order-1)/2)
  {
    subroot->elements.erase(subroot->elements.begin() + idx);
  }
  else
  {

    is_borrow_from_other = borrow_from_sibilings(subroot->parent, idx, before_idxes.back());
    if(!is_borrow_from_other && subroot->parent->elements.size() > (order-1)/2)
    {
      is_borrow_from_other = borrow_from_parent(subroot->parent, idx, before_idxes.back());
    }

    if(!is_borrow_from_other)
    {
      remove_and_reconstruct(subroot->parent, idx, before_idxes);
    }
  }
}


template <class K, class V>
void BTree<K,V>::remove_from_inner(BTreeNode* subroot, size_t idx, std::vector<size_t> before_idxes)
{
  std::cout << "remove_from_inner\n\n";
  std::cout << subroot->elements[idx].key << "\n\n";

  BTreeNode* left_max_node = subroot->children[idx];
  BTreeNode* right_min_node = subroot->children[idx+1];
  before_idxes.push_back(idx+1);

  while(!left_max_node->is_leaf)
  {
    left_max_node = left_max_node->children.back();
  }
  if(left_max_node->elements.size() > (order-1)/2)
  {
    subroot->elements[idx] = left_max_node->elements.back();
    left_max_node->elements.pop_back();
    return;
  }

  while(!right_min_node->is_leaf)
  {
    right_min_node = right_min_node->children.front();
    before_idxes.push_back(0);
  }
  if(right_min_node->elements.size() > (order-1)/2)
  {
    subroot->elements[idx] = right_min_node->elements.front();
    right_min_node->elements.erase(right_min_node->elements.begin());
    return;
  }

  else
  {
    subroot->elements[idx] = right_min_node->elements.front();
    //std::cout << "test!!!!!!!" << right_min_node->is_leaf << std::endl;
    //std::cout << before_idxes.back() << std::endl;
    remove_from_leaf(right_min_node, 0, before_idxes);
  }

  

}


template <class K, class V>
void BTree<K, V>::remove(BTreeNode* subroot, K& key, vector<size_t> before_idxes)
{
  size_t first_larger_idx = insertion_idx(subroot->elements, key);


  if (!subroot->elements.empty() && first_larger_idx < subroot->elements.size()) {
    DataPair found = subroot->elements[first_larger_idx];
    if (found.key == key)
    {
      if(subroot->is_leaf)
      {
        remove_from_leaf(subroot, first_larger_idx, before_idxes);
      }
      else
      {
        remove_from_inner(subroot, first_larger_idx, before_idxes);
      }
    }
  }

  before_idxes.push_back(first_larger_idx);

  if (!subroot->is_leaf)
  {
    remove(subroot->children[first_larger_idx], key, before_idxes);
  }
  else
  {
    return;
  }
}

/**
 * Inserts a key and value into the BTree. If the key is already in the
 * tree do nothing.
 * @param key The key to insert.
 * @param value The value to insert.
 */
template <class K, class V>
void BTree<K, V>::insert(const K& key, const V& value)
{
  /* 트리가 비어 있다면 root node를 생성한다.*/
  if (root == nullptr) {
      root = new BTreeNode(true, order);
      root->parent = nullptr;
  }

  insert(root, DataPair(key, value));
  
  /* root의 elements의 크기가 order보다 크면 새로운 root를 만들고 높이를 증가시킨다. */
  if (root->elements.size() >= order) {
      BTreeNode* new_root = new BTreeNode(false, order);
      new_root->children.push_back(root);
      split_child(new_root, 0);
      root = new_root;
      //root->parent = nullptr;
  }
}

template <class K, class V>
void BTree<K, V>::split_child(BTreeNode* parent, size_t child_idx)
{
  /** 
    * 다음의 element가 order와 같을 때 child를 split한다.
    *       |32|
    *     /      \
    * |5|8|12|   |44|
    *
    *
    * 왼쪽에 새로운 노드를 생성한다.
    *      | |32|
    *     /      \
    * |5|8|12|   |44|
    *
    * 생성된 왼쪽 노드에 split해야하는 child노드의 key기준으로 가운데 element를 가져온다.
    * parent's elements. At this point the median is still in the child.
    *      |8|32|
    *     /      \
    * |5|8|12|   |44|
    *
    * elements를 두개로 copy하여 parent와 children을 이어준다.
    *      |8|32|
    *     /  |   \
    * |5|  |12|   |44|
    *
    */

  BTreeNode* child = parent->children[child_idx];
  BTreeNode* old_child = child;
  BTreeNode* new_child = new BTreeNode(child->is_leaf, order);

  /* 
    * 1. element가 짝수인 경우
    * |5|8|12|
    * 중간 element : (3-1)/2 = 1 (전체에 1을 뺀 가운데 값)
    * 중간 자식 포인터 : 4/2 = 2 (자식의 수의 절반)
    * 
    * 2. element가 홀수인 경우
    * |8|32|
    * 중간 element : (2-1)/2 = 0 (전체에 1을 뺸 가운데 값)
    * 중간 자식 포인터 :  2/2 = 1 (전체에 절반)
    * https://www.cs.usfca.edu/~galles/visualization/BTree.html
    */
  int mid_elem_idx = (child->elements.size() - 1) / 2;
  int mid_child_idx = child->children.size() / 2;

  auto child_itr = parent->children.begin() + child_idx + 1;
  auto elem_itr = parent->elements.begin() + child_idx;
  auto mid_elem_itr = child->elements.begin() + mid_elem_idx;
  auto mid_child_itr = child->children.begin() + mid_child_idx;
  
  parent->elements.insert(elem_itr, child->elements[mid_elem_idx]);
  parent->children.insert(child_itr, new_child);

  new_child->elements.assign(mid_elem_itr + 1, child->elements.end());
  new_child->children.assign(mid_child_itr, child->children.end());
  new_child->parent = parent;
  for(auto grand_child : new_child->children)
  {
    grand_child->parent = new_child;
  }

  old_child->elements.assign(child->elements.begin(), mid_elem_itr);
  old_child->children.assign(child->children.begin(), mid_child_itr);
  old_child->parent = parent;
  for(auto grand_child : old_child->children)
  {
    grand_child->parent = old_child;
  }
}

/**
 * Private recursive version of the insert function.
 * @param subroot A reference of a pointer to the current BTreeNode.
 * @param pair The DataPair to be inserted.
 * Note: Original solution used std::lower_bound, but making the students
 * write an equivalent seemed more instructive.
 */
template <class K, class V>
void BTree<K, V>::insert(BTreeNode* subroot, const DataPair& pair)
{
  int node_insert_idx = insertion_idx(subroot->elements, pair);
 
  //element가 비어있거나, elements의 크기보다 인덱스가 작으며,
  if (!subroot->elements.empty() && node_insert_idx < subroot->elements.size()) {
    //이미 데이터가 존재한다면 insert 안함
    if (subroot->elements[node_insert_idx] == pair) return;
  }
  if (subroot->is_leaf) {
    subroot->elements.insert(subroot->elements.begin() + node_insert_idx, pair);
  } 
  else {
    BTreeNode* child = subroot->children[node_insert_idx];
    insert(child, pair);
    if(child->elements.size() >= order) split_child(subroot, node_insert_idx);
  }
}

template <class K, class V>
void BTree<K, V>::print_node(BTreeNode* node, int parent_key)
{
  for(auto it : node->elements)
    {
      std::cout << "(" << node->parent->elements.front().key << ")" << "["<< it.key << "|" << it.value << "]";
      //std::cout << "(" << parent_key << ")" << "["<< it.key << "|" << it.value << "]";
    }
    std::cout << " ";
}

template <class K, class V>
void BTree<K, V>::print(BTreeNode* subroot)
{
  vector<BTreeNode*> children;
  int parent_key;
  BTreeNode* last_child_of_generation;

  //root부터 출력
  std::cout << "print start\n";
  std::cout << "(root)" ;
  for(auto it : subroot->elements)
  {
    std::cout << "["<< it.key << "|" << it.value << "]";
   }
  std::cout << "\n";

  queue<vector<BTreeNode*>> q;
  queue<int> parent_key_q;

  if(subroot->children.size())
  {
    q.push(subroot->children);
    parent_key_q.push(subroot->elements.front().key);
    last_child_of_generation = subroot->children.back();
  }
  do
  {
    //큐에 하나를 꺼낸다음,
    children = q.front();
    q.pop();
    parent_key = parent_key_q.front();
    parent_key_q.pop();
    //자신의 값을 출력하고 자식을 삽입
    for(auto child : children)
    {
      print_node(child, parent_key);

      if(child->children.size())
      {
        q.push(child->children);
        parent_key_q.push(child->elements.front().key);
      }
      if(child == last_child_of_generation)
      {
        std::cout << "\n";
        if(child->children.size())
        {
          last_child_of_generation = child->children.back();
        }
      }
    }
  }
  while(!q.empty());

  std::cout << "print end\n";
}