#include <iostream>

template <size_t N>
class StackStorage {
private:
  size_t top_;
  alignas(std::max_align_t) char pool_[N];
public:
  StackStorage(): top_(0) {}
  StackStorage(const StackStorage&) = delete;

  void* alloc(size_t align, size_t size) {
    if (top_ % align) {
      top_ += align - top_ % align;
    }
    top_ += size;
    return pool_ + top_ - size;
  }
};


template <typename T, size_t N>
class StackAllocator {
private:
  StackStorage<N>* storage_ = nullptr;
  template<typename U, size_t Q>
  friend class StackAllocator;
public:

  using value_type = T;

  StackAllocator() = default;
  StackAllocator(const StackStorage<N>& other) : storage_(const_cast<StackStorage<N>*>(&other)) {}
  StackAllocator(const StackAllocator& other): storage_(other.storage) {}

  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other): storage_(other.storage) {}

  T* allocate(size_t size) {
    return reinterpret_cast<T*>(storage_->alloc( alignof(T), sizeof(T) * size));
  }

  void deallocate(T*, size_t) {}

  template<typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  template<typename U>
  StackAllocator& operator=(const StackAllocator<U, N>& other) {
    storage_ = other.storage;
    return *this;
  }

  template<typename U>
  bool operator==(const StackAllocator<U, N>&) {
    return true;
  }

  template<typename U>
  bool operator!=(const StackAllocator<U, N>&) {
    return false;
  }
};

template <typename T, typename allocator = std::allocator<T>>
class List {
private:
  struct Node {
  public:
    Node* prev = nullptr;
    Node* next = nullptr;
    T value;
    Node() = default;

    Node(const T& value): value(value) {}

    Node(Node* prev, Node* next, const T& value): prev(prev), next(next), value(value) {
    }

    Node(const Node& other): next(other.next), prev(other.prev) {
    }
  };

  using AllocTraits = typename std::allocator_traits<allocator>;
  using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;
  using NodeAllocTraits= typename AllocTraits::template rebind_traits<Node>;

  Node* fake_node_;
  size_t size_ = 0;
  NodeAllocator alloc_;

  void reserve(size_t size) {
    for (size_t i = 0; i < size; ++i) {
      Node* n;
      try {
        n = NodeAllocTraits::allocate(alloc_, 1);
        NodeAllocTraits::construct(alloc_, n);
      } catch (...){
        NodeAllocTraits::deallocate(alloc_, n, 1);
        throw;
      }
      ++size_;
      if (size_ == 1) {
        n->prev = fake_node_;
        n->next = fake_node_;
        fake_node_->prev = n;
        fake_node_->next = n;

      } else {
        n->prev = (end().node)->prev;
        n->next = end().node;
        (n->prev)->next = n;
        (end().node)->prev = n;
      }
    }
  }

public:
  template<bool constant>
  class Iterator {
  private:
    Node* node = nullptr;

    friend class List<T, allocator>;
  public:
    using difference_type = int;
    using value_type = typename std::conditional<constant, const T, T>::type;
    using iterator_traits = std::iterator_traits<Iterator<constant>>;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = typename std::conditional<constant, const T*, T*>::type;
    using reference = typename std::conditional<constant, const T&, T&>::type;


    Iterator() = default;

    Iterator(Node* node): node(node) {};

    Iterator(const Iterator<false>& other): node(other.node) {};


    Iterator& operator=(const Iterator<false> &other) {
      node = other.node;
      return *this;
    }

    Iterator& operator++() {
      node = node->next;
      return *this;
    }

    Iterator operator++(int) {
      Iterator iterator = *this;
      ++(*this);
      return iterator;
    }

    Iterator& operator--() {
      node = node->prev;
      return *this;
    }

    Iterator operator--(int) {
      Iterator iterator = *this;
      --(*this);
      return iterator;
    }

    bool operator==(const Iterator<constant>& other) const {
      return node == other.node;
    }

    bool operator!=(const Iterator<constant>& other) const {
      return node != other.node;
    }


    reference operator*() const {
      return node->value;
    }


    pointer operator->() const {
      return &(node->value);
    }

  };


  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  List(): alloc_(NodeAllocator())  {
    try {
      fake_node_ = NodeAllocTraits::allocate(alloc_, 1);
    } catch(...) {
      NodeAllocTraits::deallocate(alloc_, fake_node_, 1);
      throw;
    }
    fake_node_->prev = fake_node_;
    fake_node_->next = fake_node_;
  }

  List(size_t size): List() {
    reserve(size);
  }

  List(size_t size, const T& value): List() {
    for (size_t i = 0; i < size; ++i) {
      push_back(value);
    }
  }

  List(const allocator& a): alloc_(NodeAllocTraits::select_on_container_copy_construction(a))
  {
    fake_node_ = NodeAllocTraits::allocate(alloc_, 1);
    fake_node_->prev = fake_node_;
    fake_node_->next = fake_node_;
  }

  List(size_t size, const allocator& alloc): List(alloc) {
    reserve(size);
  }


  List(size_t size, const T& value, const allocator& alloc): List(alloc) {
    for (size_t i = 0; i < size; ++i) {
      push_back(value);
    }
  }

  allocator get_allocator() {
    return alloc_;
  }

  List(const List& other): List(NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
    Node* n = other.fake_node_;
    for(size_t i = 0; i < other.size_; ++i) {
      n = n->next;
      try {
        push_back(n->value);
      } catch(...) {
        for (size_t j = 0; j < i; ++j) {
          pop_back();
        }
        throw;
      }
    }
  }

  List& operator=(const List& other) {


    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc_ = other.alloc;
    }

    Node* n = other.fake_node_;
    for(size_t i = 0; i < other.size_; ++i) {
      n = n->next;
      try {
        push_back(n->value);
      } catch(...) {
        for (size_t j = 0; j < i; ++j) {
          pop_back();
        }
        throw;
      }
    }

    while(size_ > other.size()) {
      pop_back();
    }


    return *this;
  }

  size_t size() const {
    return size_;
  }

  void push_back(const T& value) {
    insert(end(), value);
  }

  void push_front(const T& value) {
    insert(begin(), value);
  }

  void pop_back() {
    iterator iter = --end();
    erase(iter);
  }

  void pop_front() {
    erase(begin());
  }

  iterator begin() const {
    return iterator(fake_node_->next);
  }

  iterator end() const {
    return iterator(fake_node_);
  }

  const_iterator cbegin() const {
    return const_iterator(fake_node_->next);
  }

  const_iterator cend() const {
    return const_iterator(fake_node_);
  }

  reverse_iterator rbegin() const {
    return reverse_iterator(fake_node_);
  }

  reverse_iterator rend() const {
    return reverse_iterator(fake_node_);
  }

  const_reverse_iterator rcbegin() const {
    return const_reverse_iterator(fake_node_);
  }

  const_reverse_iterator rcend() const {
    return const_reverse_iterator(fake_node_);
  }

  void insert(const_iterator iter, const T& value) {
    Node* n;
    try {
      n = NodeAllocTraits::allocate(alloc_, 1);
      NodeAllocTraits::construct(alloc_, n, value);
    } catch (...){
      NodeAllocTraits::deallocate(alloc_, n, 1);
      throw;
    }
    ++size_;
    if (size_ == 1) {
      n->prev = fake_node_;
      n->next = fake_node_;
      fake_node_->prev = n;
      fake_node_->next = n;

    } else {
      n->prev = (iter.node)->prev;
      n->next = iter.node;
      (n->prev)->next = n;
      (iter.node)->prev = n;
    }
  }

  void erase(const_iterator iter) {
    --size_;
    if (size_ == 0) {
      fake_node_ -> prev = fake_node_;
      fake_node_ -> next = fake_node_;

    } else {
      ((iter.node)->prev)->next = iter.node->next;
      ((iter.node)->next)->prev = iter.node->prev;
    }
    NodeAllocTraits::destroy(alloc_, iter.node);
    NodeAllocTraits::deallocate(alloc_, iter.node, 1);
  }

  ~List() {
    while (size_) {
      pop_back();
    }
    NodeAllocTraits::deallocate(alloc_, fake_node_, 1);
  }
};

