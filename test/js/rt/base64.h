#define ENCODE 0
#define DECODE 1
static char values[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static char *getMem(unsigned long inputSize, char value) {
  return (char*)malloc(1 + (value == ENCODE) ? ((inputSize * 4) / 3) : ((inputSize * 3) / 4));
}

char *encode(const char *input) {
  unsigned long len = strlen(input), i;
  char *res = getMem(len, ENCODE), *p = res, b;

  for (i = 0; i < len; i += 3) {
    b = (input[i] & 0xFC) >> 2;
    *p++ = values[b];
    b = (input[i] & 0x03) << 4;

    if (i + 1 < len) {
      b |= ((input[i + 1] & 0xf0) >> 4);
      *p++ = values[b];
      b = (input[i + 1] & 0x0F) << 2;

      if (i + 2 < len) {
        b |= (input[i + 2] & 0xC0) >> 6;
        *p++ = values[b];
        b = (input[i + 2] & 0x3F);
        *p++ = values[b];
      } else {
        *p++ = values[b];
        *p++ = values[64];
        *p++ = values[64];
      }
    } else {
      *p++ = values[b];
      *p++ = values[64];
    }
  }

  *p = '\0';
  return res;
}

