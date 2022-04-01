#ifndef MEMLEAKDETECTOR_MEMLEAKDETECTOR_H
#define MEMLEAKDETECTOR_MEMLEAKDETECTOR_H

#include <iostream>

// 内存申请记录
struct mem_list_t {
  mem_list_t *next;
  mem_list_t *prev;
  std::size_t size; // 实际申请的内存大小
  const char *filename; // 文件名
  unsigned int line; // 代码行

  mem_list_t() {
    next = this;
    prev = this;
  }
};

// 已申请的内存量
std::size_t total_alloc_size = 0;
// 所有内存申请记录的链表，是个栈对象
mem_list_t mem_list_head;
// 内存申请记录的大小
#define MEM_LIST_ITEM_SIZE sizeof(mem_list_t)

// 处理实际的内存申请
void *alloc_mem(std::size_t size, const char *file, int line, bool is_array) {
  // 在需要申请的内存前边多申请一块内存放置内存申请记录
  std::size_t s = size + MEM_LIST_ITEM_SIZE;
  auto *p = (mem_list_t *) malloc(s);
  if (!p) {
    std::cout << "Out of memory when allocating " << size
              << "bytes" << std::endl;
    abort();
  }
  // 记录申请的内存信息
  p->line = line;
  p->filename = file;
  p->size = size;
  total_alloc_size += size;
  // 加入链表
  p->prev = &mem_list_head;
  p->next = mem_list_head.next;
  mem_list_head.next->prev = p;
  mem_list_head.next = p;

  return (void *) ((unsigned long) p + MEM_LIST_ITEM_SIZE);
}

/**
 * 重载运算符 new，记录内存申请
 * @param file_name 申请所在文件名
 * @param line 申请所在代码行
 */
void *operator new(std::size_t size, const char *file_name, int line) {
  return alloc_mem(size, file_name, line, false);
}

void *operator new[](std::size_t size, const char *file_name, int line) {
  return alloc_mem(size, file_name, line, true);
}

/**
 * 对于内部库（如智能指针）等不被 new 宏替换的地方，重定义默认的 new 运算符
 * 默认的运算符是弱符号，不会发生重复定义错误
 *
 * 这种方式不能记录到内存申请发生的位置
 * TODO 可以考虑记录内存申请的函数调用栈
 */
void* operator new(std::size_t size) {
  return operator new(size, nullptr, 0);
}

void free_mem(void *addr, bool is_array) {
  if (!addr) return;

  auto p = (mem_list_t *) ((unsigned long) addr - MEM_LIST_ITEM_SIZE);
  total_alloc_size -= p->size;

  // 从链表中删除
  p->prev->next = p->next;
  p->next->prev = p->prev;

  free(p); // 释放内存
}

void operator delete(void *ptr) noexcept {
  free_mem(ptr, false);
}

void operator delete[](void *ptr) noexcept {
  free_mem(ptr, true);
}

void checkLeaks() {
  if (total_alloc_size == 0) return;
  int leak_cnt = 0;
  auto p = mem_list_head.next;
  // 遍历内存申请记录链表，输出内存泄漏信息
  while (p != &mem_list_head) {
    auto leak_addr = (void *) ((unsigned long) p - MEM_LIST_ITEM_SIZE);
    std::cout << "Leaked object at " << leak_addr << ", size: "
              << p->size << " in " << (p->filename ? p->filename : "null")
              << ":" << p->line << std::endl;
    p = p->next;
    leak_cnt++;
  }
  std::cout << leak_cnt << " leaks found" << std::endl;
}

// 用宏替换 new，使得显式使用 new 时自动调用重载的 new 记录下本次内存申请
#define new new(__FILE__, __LINE__)

#endif //MEMLEAKDETECTOR_MEMLEAKDETECTOR_H
