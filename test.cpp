#include "memLeakDetector.h"
#include <memory>

using namespace std;

struct A { int a; };

int main() {
  // 基本类型
  int *a = new int;
  delete a;
  // 对象
  auto aa = new A();
  // 智能指针
  {
    auto sp = make_shared<A>();
  }

  // 触发内存泄漏检查
  checkLeaks();
  return 0;
}
