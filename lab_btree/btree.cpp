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
  /* TODO Your code goes here! */
  size_t first_larger_idx = insertion_idx(subroot->elements, key);

  /* If first_larger_idx is a valid index and the key there is the key we
  * are looking for, we are done. */
  /* Otherwise, we need to figure out which child to explore. For this we
  * can actually just use first_larger_idx directly. E.g.
  * | 1 | 5 | 7 | 8 |
  * Suppose we are looking for 6. first_larger_idx is 2. This means we want to
  * explore the child between 5 and 7. The children vector has a pointer for
  * each of the horizontal bars. The index of the horizontal bar we want is
  * 2, which is conveniently the same as first_larger_idx. If the subroot is
  * a leaf and we didn't find the key in it, then we have failed to find it
  * anywhere in the tree and return the default V.
  */

  if (!subroot->elements.empty() && first_larger_idx < subroot->elements.size()) {
    DataPair found = subroot->elements[first_larger_idx];
    if (found.key == key) return found.value;
  }

  if (!subroot->is_leaf) return find (subroot->children[first_larger_idx], key);
  else return V();
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
  }
  insert(root, DataPair(key, value));
  
  /* elements의 크기가 order보다 크면 높이를 증가시킨다. */
  if (root->elements.size() >= order) {
      BTreeNode* new_root = new BTreeNode(false, order);
      new_root->children.push_back(root);
      split_child(new_root, 0);
      root = new_root;
  }
}

/**
 * Splits a child node of a BTreeNode. Called if the child became too
 * large.
 * @param parent The parent whose child we are trying to split.
 * @param child_idx The index of the child in its parent's children
 * vector.
 */
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

  /* Iterator for where we want to insert the new child. */
  auto child_itr = parent->children.begin() + child_idx + 1;
  /* Iterator for where we want to insert the new element. */
  auto elem_itr = parent->elements.begin() + child_idx;
  /* Iterator for the middle element. */
  auto mid_elem_itr = child->elements.begin() + mid_elem_idx;
  /* Iterator for the middle child. */
  auto mid_child_itr = child->children.begin() + mid_child_idx;

  /* TODO Your code goes here! */
  parent->elements.insert(elem_itr, child->elements[mid_elem_idx]);
  parent->children.insert(child_itr, new_right);

  new_child->elements.assign(mid_elem_itr + 1, child->elements.end());
  new_child->children.assign(mid_child_itr, child->children.end());

  old_child->elements.assign(child->elements.begin(), mid_elem_itr);
  old_child->children.assign(child->children.begin(), mid_child_itr);
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
  } else {
    BTreeNode* child = subroot->children[node_insert_idx];
    insert(child, pair);
    if(child->elements.size() >= order) split_child(subroot, node_insert_idx);
  }
}

template <class K, class V>
void BTree<K, V>::print_node(BTreeNode* node)
{
  for(auto it : node->elements)
    {
      std::cout << "["<< it.key << "|" << it.value << "]";
    }
    std::cout << " ";
}

template <class K, class V>
void BTree<K, V>::print(BTreeNode* subroot)
{
  vector<BTreeNode*> children;
  
  std::cout << "print start\n";
  print_node(subroot);
  std::cout << "\n";

  queue<vector<BTreeNode*>> q;
  if(subroot->children.size())
  {
    q.push(subroot->children);
  }
  do
  {
    //큐에 하나를 꺼낸다음,
    children = q.front();
    q.pop();
    //자신의 값을 출력하고 자식을 삽입
    for(auto it : children)
    {
      print_node(it);
      if(it->children.size())
      {
        q.push(it->children);
      }
    }
    std::cout << "\n";
  }
  while(!q.empty());

  std::cout << "print end\n";
}