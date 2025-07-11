#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
  size_t i = 1;
  EXPECT(c, literal[0]);
  for (; literal[i] != '\0'; i++) {
    if (c->json[i-1] != literal[i]) {
      return LEPT_PARSE_INVALID_VALUE;
    }
  }
  c->json += i - 1;
  v->type = type;
  return LEPT_PARSE_OK;
}

/*number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。
 * 只有整数是必需部分。注意和直觉可能不同的是，正号是不合法的。
 * 整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）。
 * 也就是说，0123 不是一个合法的 JSON 数字。
 * 小数部分比较直观，就是小数点后是一或多个数字（0-9）。
 * JSON 可使用科学记数法，指数部分由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。
 * */
static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;

    /* 可选负号 */
    if (*p == '-') p++;

    /* 整数部分 */
    if (*p == '0') {
        p++;
        /* 0 后不能接数字（如 0123 不合法） */
        /* 0后只能接小数点，e E 或者啥也不接 */
        if (*p != '.' && *p != 'e' && *p != 'E' && *p != '\0') {
          return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    } else if (ISDIGIT1TO9(*p)) {
        p++;
        while (ISDIGIT(*p)) p++;
    } else {
        return LEPT_PARSE_INVALID_VALUE;
    }
    /* 上边处理数字部分，例如1234.1234，现在*p = '.'，进入到这里 */
    /* 小数部分（可选） */
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE; /* 小数点后必须有数字 */
        p++;
        while (ISDIGIT(*p)) p++;
    }

    /* 指数部分（可选） */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++; /* 指数可选符号 */
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE; /* 指数必须有数字 */
        p++;
        while (ISDIGIT(*p)) p++;
    }

    /* 必须匹配完整数字，且不能有额外字符 */
    if (p == c->json) return LEPT_PARSE_INVALID_VALUE;

    /* 现在可以安全转换为 double */
    char* end;
    v->n = strtod(c->json, &end);

    if (v->n == HUGE_VAL || v->n == -HUGE_VAL) {
      return LEPT_PARSE_NUMBER_TOO_BIG;
    }

    /* 如果 c->json == end，表示没有进行任何有效的数字转换，
     * 返回错误码 LEPT_PARSE_INVALID_VALUE。*/
    if (c->json == end)
      return LEPT_PARSE_INVALID_VALUE;


    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}



static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
