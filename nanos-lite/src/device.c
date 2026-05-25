#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

#define KEYDOWN_MASK 0x8000

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  if (key != _KEY_NONE) {
    int is_keydown = key & KEYDOWN_MASK;
    int keycode = key & ~KEYDOWN_MASK;
    int n = snprintf(buf, len, "%s %s\n", is_keydown ? "kd" : "ku", keyname[keycode]);
    return n < 0 ? 0 : (n < len ? n : len);
  }

  static unsigned long last = 0;
  unsigned long now = _uptime();
  if (now / 33 != last / 33) {
    last = now;
    int n = snprintf(buf, len, "t %lu\n", now);
    return n < 0 ? 0 : (n < len ? n : len);
  }

  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  const uint32_t *pixels = (const uint32_t *)buf;
  int pos = offset / sizeof(uint32_t);
  int n = len / sizeof(uint32_t);

  while (n > 0) {
    int x = pos % _screen.width;
    int y = pos / _screen.width;
    int cnt = _screen.width - x;
    if (cnt > n) { cnt = n; }
    _draw_rect(pixels, x, y, cnt, 1);
    pixels += cnt;
    pos += cnt;
    n -= cnt;
  }
  _draw_sync();
}

void init_device() {
  _ioe_init();
  snprintf(dispinfo, sizeof(dispinfo), "WIDTH : %d\nHEIGHT: %d\n", _screen.width, _screen.height);
}
