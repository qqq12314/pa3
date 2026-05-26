#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

#define KEYDOWN_MASK 0x8000

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  static char evbuf[64];
  static size_t ev_len = 0;
  static size_t ev_pos = 0;

  if (len == 0) {
    return 0;
  }

  // FILE/getc() may call read(fd, ..., 1). Therefore /dev/events must behave
  // like a byte stream: keep the current event line and return it piece by piece.
  if (ev_pos >= ev_len) {
    int key = _read_key();

    if (key != _KEY_NONE) {
      int is_keydown = key & KEYDOWN_MASK;
      int keycode = key & ~KEYDOWN_MASK;
      ev_len = snprintf(evbuf, sizeof(evbuf), "%s %s\n",
          is_keydown ? "kd" : "ku", keyname[keycode]);
    } else {
      unsigned long now = _uptime();
      ev_len = snprintf(evbuf, sizeof(evbuf), "t %lu\n", now);
    }

    if (ev_len >= sizeof(evbuf)) {
      ev_len = sizeof(evbuf) - 1;
      evbuf[ev_len] = '\0';
    }
    ev_pos = 0;
  }

  size_t n = ev_len - ev_pos;
  if (n > len) {
    n = len;
  }
  memcpy(buf, evbuf + ev_pos, n);
  ev_pos += n;
  return n;
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
