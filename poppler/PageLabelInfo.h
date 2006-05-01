#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include <goo/gtypes.h>
#include <goo/GooList.h>
#include <goo/GooString.h>
#include <Object.h>

class PageLabelInfo {
public:
  PageLabelInfo(Object *tree, int numPages);
  ~PageLabelInfo();
  GBool labelToIndex(GooString *label, int *index);
  GBool indexToLabel(int index, GooString *label);

private:
  void parse(Object *tree);

private:
  struct Interval {
    Interval(Object *dict, int baseA);
    ~Interval();
    GooString *prefix;
    enum NumberStyle {
      None,
      Arabic,
      LowercaseRoman,
      UppercaseRoman,
      UppercaseLatin,
      LowercaseLatin
    } style;
    int first, base, length;
  };

  GooList intervals;
};
static void toRoman(int number, GooString *str, GBool uppercase) {
  static const char uppercaseNumerals[] = "IVXLCDM";
  static const char lowercaseNumerals[] = "ivxlcdm";
  int divisor;
  int i, j, k;
  const char *wh;

  if (uppercase)
    wh = uppercaseNumerals;
  else
    wh = lowercaseNumerals;

  divisor = 1000;
  for (k = 3; k >= 0; k--) {
    i = number / divisor;
    number = number % divisor;

    switch (i) {
    case 0:
      break;
    case 5:
      str->append(wh[2 * k + 1]);
      break;
    case 9:
      str->append(wh[2 * k + 0]);
      str->append(wh[ 2 * k + 2]);
      break;
    case 4:
      str->append(wh[2 * k + 0]);
      str->append(wh[2 * k + 1]);
      break;
    default:
      if (i > 5) {
	str->append(wh[2 * k + 1]);
	i -= 5;
      }
      for (j = 0; j < i; j++) {
	str->append(wh[2 * k + 0]);
      }
    }
	
    divisor = divisor / 10;
  }
}