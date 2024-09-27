#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "tools/random_engine.h"
#include "tools/static_reflection.h"
namespace HoshinoDB {
constexpr static int MAX_LEVEL = 16;

template <class T, class Comp>
class SkipList {
 private:
  struct Node;
  class Iterator;
  // using NodeType = Node<T>;
  using NodePtrVec = std::vector<std::shared_ptr<Node>>;

 public:
  SkipList(Comp cmp) : rnd(0.0, 1.0), levelCount_(1), cmp_(cmp) {
    head_ = std::make_shared<Node>();
    static_assert(can_compare_with<Comp, T>::value == true,
                  R"(SkipList: Comparator can not 
            compare the "T" elems)");
  }
  auto Find(T target) const -> std::shared_ptr<Node> {
    auto p = this->head_;
    for (int i = levelCount_ - 1; i >= 0; --i) {
      while (GetForwardNode(p, i) &&
             cmp_.Compare(GetForwardNode(p, i)->data_, target) < 0) {
        p = GetForwardNode(p, i);
      }
    }
    auto next = GetForwardNode(p, 0);
    if (next != nullptr && cmp_.Compare(next->data_, target) == 0) {
      return next;
    }
    return nullptr;
  }
  auto Insert(T value) -> void {
    auto x = this->Find(value);
    assert(x == nullptr &&
           "SkipList: there already exist a same value in skiplist");

    const auto level = GenerateRandomLevel();
    auto newNode = std::make_shared<Node>(value, level);
    NodePtrVec temp{level, this->head_};
    // 找到newNode在各个层级的前置节点
    for (int i = level - 1; i >= 0; --i) {
      while (GetForwardNode(temp[i], i) &&
             cmp_.Compare(GetForwardNode(temp[i], i)->data_, value) < 0) {
        temp[i] = GetForwardNode(temp[i], i);
      }
    }
    // 更改newNode的forward
    for (int i = level - 1; i >= 0; --i) {
      newNode->forwards[i] = GetForwardNode(temp[i], i);
      temp[i]->forwards[i] = newNode;
    }
    if (levelCount_ < level) levelCount_ = level;
  }
  auto Delete(T value) -> bool {
    NodePtrVec temp{static_cast<std::size_t>(this->levelCount_), this->head_};
    for (int i = this->levelCount_ - 1; i >= 0; --i) {
      while (GetForwardNode(temp[i], i) &&
             cmp_.Compare(GetForwardNode(temp[i], i)->data_, value) < 0) {
        temp[i] = GetForwardNode(temp[i], i);
      }
    }
    // 此时temp[0](跳表最底层)的下一个应为大于等于value的节点
    auto target = GetForwardNode(temp[0], 0);
    if (target != nullptr && target->data_ == value) {  // 存在要删除的节点
      for (int i = this->levelCount_ - 1; i >= 0; --i) {
        auto forwardNode = GetForwardNode(temp[i], i);
        // 将该节点的各层索引一路删除
        if (forwardNode != nullptr && forwardNode->data_ == value) {
          temp[i]->forwards[i] = GetForwardNode(GetForwardNode(temp[i], i), i);
        }
      }
      // 调整levelCount_
      while (this->levelCount_ > 1 &&
             GetForwardNode(this->head_, this->levelCount_) == nullptr) {
        this->levelCount_--;
      }
      return true;
    }
    return false;
  }

  auto GetIterator() -> std::unique_ptr<Iterator> {
    return std::make_unique<Iterator>(this);
  }

 private:
  RandomEngine<float> rnd;
  std::shared_ptr<Node> head_;
  int levelCount_;  // 当前跳表的最大层数
  Comp cmp_;
  std::uint8_t GenerateRandomLevel() {
    int level = 1;
    constexpr float SKIPLIST_P = 0.5;
    while (rnd.GetRandom() < SKIPLIST_P && level < MAX_LEVEL) {
      level++;
    }
    return level;
  }
  auto GetForwardNode(std::shared_ptr<Node> p,
                      int level) const -> std::shared_ptr<Node> {
    return p->forwards[level];
  }
  class Iterator {
   public:
    Iterator(SkipList *list)
        : list_(list), prev_(list_->head_), cur_(list_->head_){};

    auto Next() -> std::shared_ptr<Node> const {
      auto next = list_->GetForwardNode(cur_, 0);
      if (cur_ != list_->head_) prev_ = cur_;
      cur_ = next;
      return next;
    }

    auto Prev() -> std::shared_ptr<Node> const { return prev_; }

    auto Seek(const T &target) -> bool {
      auto p = list_->head_;
      for (int i = list_->levelCount_ - 1; i >= 0; --i) {
        while (list_->GetForwardNode(p, i) &&
               list_->cmp_.Compare(list_->GetForwardNode(p, i)->data_, target) <
                   0) {
          p = list_->GetForwardNode(p, i);
        }
      }
      auto next = list_->GetForwardNode(p, 0);
      if (next != nullptr && next->data_ == target) {
        prev_ = p;
        cur_ = next;
        return true;
      }
      return false;
    }

   private:
    SkipList *list_;
    std::shared_ptr<Node> prev_;
    std::shared_ptr<Node> cur_;
  };
};

template <class T, class Comparator>
struct SkipList<T, Comparator>::Node {
  // shared_ptr里的Node指代的是实例化后的Node<T>，Node后面加不加<T>都可以
  using NodePtrVec = std::vector<std::shared_ptr<Node>>;
  T data_;
  NodePtrVec forwards;
  int maxLevel_;
  Node() : data_(T{}), forwards{MAX_LEVEL, nullptr}, maxLevel_(0) {}
  Node(T data, int maxLevel)
      : data_(data), forwards{MAX_LEVEL, nullptr}, maxLevel_(maxLevel) {}
};

}  // namespace HoshinoDB
