#ifndef LEPTJSON_H__
#define LEPTJSON_H__

/*
 * JSON 中有 6 种数据类型
 * 如果把 true 和 false 当作两个类型就是 7 种
 * 我们为此声明一个枚举类型（enumeration type）
 * */
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

/* JSON 是一个树形结构，我们最终需要实现一个树的数据结构，
 * 每个节点使用 lept_value 结构体表示，我们会称它为一个 JSON 值（JSON value）。
 * */
typedef struct {
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

int lept_parse(lept_value* v, const char* json);

lept_type lept_get_type(const lept_value* v);

#endif /* LEPTJSON_H__ */
