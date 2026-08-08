/*
 * =============================================================================
 * 设计模式：享元模式（Flyweight Pattern）—— 文本格式化
 * =============================================================================
 *
 * 【一句话概括】
 * 将格式化信息与文本内容分离，使用范围对象（Range）描述格式，而非为每个字符存储格式标志。
 *
 * 【适用场景 —— 通用】
 * - 需要对大段文本应用少量格式化规则时，避免 O(n) 空间复杂度
 *
 * 【金融工程应用】
 * - 回测标记管理：对长时间序列（数百万个 K 线）标记少量交易信号（买入/卖出点），
 *   使用范围描述（买入区间/卖出区间）替代逐 Bar 标记，节省大量内存
 * - 行情区间标注：对 Tick 数据流标记盘前/盘中/盘后时段，用三个范围对象替代逐 Tick 标记
 * - 复权因子应用：对日线序列标记除权除息日范围，仅在特定区间应用复权因子
 *
 * 【本示例说明】
 * BetterFormattedText 只存储格式化范围（TextRange），输出时判断字符是否在范围内，
 * 内存开销 O(k) vs 朴素方案的 O(n)。
 */

#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ============================================================================
// 朴素方案：FormattedText
// 为每个字符分配一个 bool 数组来标记格式化状态
// ============================================================================
class FormattedText {
  string plain_text;

  // 朴素实现：使用 bool 数组标记每个字符是否大写
  // 文本越长，数组越大，内存浪费严重
  bool *caps;

public:
  explicit FormattedText(const string &plain_text) : plain_text(plain_text) {
    caps = new bool[plain_text.length()];
    memset(caps, 0, plain_text.length());
  }

  virtual ~FormattedText() { delete caps; }

  // 将指定范围内的字符标记为大写
  void capitalize(int start, int end) {
    for (int i = start; i <= end; ++i) {
      caps[i] = true;
    }
  }

  friend ostream &operator<<(ostream &os, const FormattedText &obj) {
    string s;
    for (size_t i = 0; i < obj.plain_text.length(); ++i) {
      char c = obj.plain_text[i];
      s += (obj.caps[i] ? toupper(c) : c);
    }
    return os << s;
  }
};

// ============================================================================
// 享元方案：BetterFormattedText
// 使用 TextRange 范围描述格式信息，避免逐字符存储
// ============================================================================
class BetterFormattedText {
public:
  // 格式化范围：描述一段文本的格式信息
  // 这是享元模式中的"外在状态"——轻量级、可共享
  struct TextRange {
    int start, end;
    bool capitalize; // 可扩展：bold, italic, underline, ...

    // 判断给定位置是否在此范围内
    bool covers(int position) const {
      return position >= start && position <= end;
    }
  };

  // 获取一个格式化范围引用，可在外部设置其属性
  TextRange &get_range(int start, int end) {
    formatting.emplace_back(TextRange{start, end});
    return *formatting.rbegin();
  }

  BetterFormattedText(const string &plain_text) : plain_text(plain_text) {}

  friend ostream &operator<<(ostream &os, const BetterFormattedText &obj) {
    string s;
    for (size_t i = 0; i < obj.plain_text.length(); ++i) {
      auto c = obj.plain_text[i];
      // 遍历所有格式化范围，检查当前位置是否被覆盖
      for (const auto &rng : obj.formatting) {
        if (rng.covers(i) && rng.capitalize) {
          c = toupper(c);
        }
        s += c;
      }
    }
    return os << s;
  }

private:
  string plain_text;
  // 只存储格式化范围，而非逐字符的标志位
  // 当格式化规则远少于文本长度时，内存效率极高
  vector<TextRange> formatting;
};

int main() {
  // 朴素方案：每个字符一个 bool
  FormattedText ft("this is a brave new world");
  ft.capitalize(10, 15);
  cout << ft << endl;

  // 享元方案：只存储范围信息
  BetterFormattedText bft("this is a brave new world");
  bft.get_range(10, 15).capitalize = true;
  cout << bft << endl;

  return 0;
}
