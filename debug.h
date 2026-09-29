#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG_EXPR(expr) fprintf(stderr, "%s:%d:%s(): %s: 0x%llX\n", __FILE__, __LINE__, __func__, #expr, (unsigned long long)(expr))
#define DEBUG_STR(expr) fprintf(stderr, "%s:%d:%s(): %s: %s\n", __FILE__, __LINE__, __func__, #expr, (char*)(expr))

#define DEBUG_PRINT(str) fprintf(stderr, "%s:%d:%s(): %s\n", __FILE__, __LINE__, __func__, str);

#endif //DEBUG_H
