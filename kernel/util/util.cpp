#include <util.h>

template<typename T>
void KernelUtil::Swap(T& a, T& b)
{
    T c;
    c = a;
    a = b;
    b = c;
}

u32 KernelUtil::Strlen(const char* s) {
    u32 c = 0;
    for (const char *pt = s; *pt != '\0'; ++pt, ++c);
    return c;
}

void* KernelUtil::Memset(void *dst, int c, u32 n)
{
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

int KernelUtil::Memcmp(const void *v1, const void *v2, u32 n)
{
  const u8 *s1, *s2;

  s1 = (const unsigned char*)v1;
  s2 = (const unsigned char*)v2;
  while(n-- > 0){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}

void* KernelUtil::Memmove(void *dst, const void *src, u32 n)
{
  const char *s;
  char *d;

  if(n == 0)
    return dst;

  s = (const char*)src;
  d = (char *)dst;
  if(s < d && s + n > d){
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
  } else
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void* KernelUtil::Memcpy(void *dst, const void *src, u32 n)
{
  return Memmove(dst, src, n);
}

int KernelUtil::Strncmp(const char *p, const char *q, u32 n)
{
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (u8)*p - (u8)*q;
}

char* KernelUtil::Strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char* KernelUtil::Safestrcpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  if(n <= 0)
    return os;
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

char* my_strchr(const char* str, char ch) {
    while (*str != '\0') {
        if (*str == ch) {
            return (char*)str;
        }
        str++;
    }
    if (*str == ch) {
        return (char*)str;
    }
    return (char *)0;
}

char KernelUtil::ToUpper(char c) {
  /* 判断是否为小写字母，如果是，转换成大写字母 */
  if(c >= 'a' && c <= 'z') {
    return c - 'a' + 'A';
  }
  /* 如果不是小写字母，则原样返回 */
  return c;
}

int KernelUtil::IsLower(int c) {
  /* 如果是小写字母，返回非0值，否则返回0 */
  if(c >= 'a' && c <= 'z') {
    return 1;
  }
  return 0;
}

u64 getCPUCFG(u64 input) {
    u64 rel;
    __asm__(
        "cpucfg %0, %1"
        :"=r"(rel)
        :"r"(input)
        :
    );
    return rel;
}
