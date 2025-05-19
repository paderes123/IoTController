#include "TextWrapper.h"
#include <string.h>

TextWrapper::TextWrapper(U8G2& display) : u8g2(display) {}

void TextWrapper::WrapAndDisplayText(const char* text, uint8_t x, uint8_t startY, uint8_t lineHeight) {
  const uint16_t maxWidth = 128;
  char buffer[256];
  char line[256] = "";
  int y = startY; // test

  u8g2.setFont(u8g2_font_ncenB08_tr);

  while (*text) {
    int i = 0;
    while (*text && *text != ' ' && i < sizeof(buffer) - 1) {
      buffer[i++] = *text++;
    }
    buffer[i] = '\0';
    if (*text == ' ') text++;

    char testLine[256];
    strcpy(testLine, line);
    strcat(testLine, buffer);
    strcat(testLine, " ");

    uint16_t w = u8g2.getStrWidth(testLine);
    if (w > maxWidth) {
      u8g2.drawStr(x, y, line);
      y += lineHeight;
      strcpy(line, buffer);
      strcat(line, " ");
    } else {
      strcpy(line, testLine);
    }
  }

  if (strlen(line) > 0) {
    u8g2.drawStr(x, y, line);
  }
}
