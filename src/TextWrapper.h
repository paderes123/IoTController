#ifndef TEXTWRAPPER_H
#define TEXTWRAPPER_H

#include <U8g2lib.h>

class TextWrapper {
public:
  TextWrapper(U8G2& display);
  void WrapAndDisplayText(const char* text, uint8_t x, uint8_t startY, uint8_t lineHeight);

private:
  U8G2& u8g2;
};

#endif
