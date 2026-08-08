/*
 * =============================================================================
 * 设计模式：代理模式（Proxy Pattern）—— 虚拟代理（懒加载）
 * =============================================================================
 *
 * 【一句话概括】
 * 延迟创建开销大的对象，直到真正需要时才进行实例化（Lazy Initialization）。
 *
 * 【适用场景 —— 通用】
 * - 对象的创建成本很高（如加载大图片、建立数据库连接），且不一定会被立即使用时
 *
 * 【金融工程应用】
 * - 历史数据懒加载：策略初始化时声明需要的数据范围，但只有在策略真正开始计算时
 *   才从数据库/文件加载数据，避免预加载全部历史数据造成启动延迟和内存浪费
 * - 定价模型懒加载：风险管理系统中几十个定价模型，只在对应产品首次被估值时才初始化，
 *   避免启动时加载所有模型
 * - 回测结果延迟计算：回测引擎只在用户查看某个绩效指标时才计算它（如用户点了"最大回撤"按钮
 *   才计算最大回撤），避免一次性计算所有指标
 * - 因子值懒计算：因子库中数百个因子，只在策略引用时才计算，避免不必要的 CPU 消耗
 *
 * 【本示例说明】
 * LazyBitmap 构造时不加载图片，首次 draw() 时才创建 Bitmap 并加载。
 */

#include <iostream>
#include <string>

using namespace std;

// 抽象主题（Subject）：定义代理和真实对象的公共接口
struct Image {
  virtual void draw() = 0;
};

// ============================================================================
// 真实主题（Real Subject）：Bitmap
// 构造时立即加载图片文件，成本较高
// ============================================================================
struct Bitmap : Image {
  Bitmap(const string &filename) {
    cout << "Loading bitmap from " << filename << endl;
  }

  void draw() override { cout << "Drawing bitmap" << endl; }
};

// ============================================================================
// 虚拟代理（Virtual Proxy）：LazyBitmap
// 构造时不加载图片，只在首次 draw() 调用时才延迟创建 Bitmap
// ============================================================================
struct LazyBitmap : Image {
  string filename;
  Bitmap *bmp{nullptr};  // 初始为 nullptr，表示尚未加载

  LazyBitmap(const string &filename) : filename(filename) {
    cout << "Loading Lazy" << endl;
  }

  // 懒加载：首次调用 draw() 时才创建真正的 Bitmap 对象
  void draw() override {
    cout << "Drawing Lazy" << endl;

    // 如果尚未加载，则创建 Bitmap（懒加载的核心逻辑）
    if (!bmp)
      bmp = new Bitmap(filename);
    // 委托给真实对象执行绘制
    bmp->draw();
  }
};

int main() {
  // 真实对象：构造时立即加载
  Bitmap bmp{"image.png"};
  bmp.draw();
  cout << endl;

  // 虚拟代理：构造时只记录文件名，不加载图片
  LazyBitmap lazy_bmp{"image.png"};
  // 首次 draw() 才真正加载和绘制
  lazy_bmp.draw();

  return 0;
}
