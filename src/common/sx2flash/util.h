#ifndef UTIL_H
#define UTIL_H

#define when_true(cond, where) do { if (cond) goto where; } while (0)
#define when_false(cond, where) when_true(!(cond), where)
#define when_null(expr, where) when_true((expr) == 0, where)

#define minimum(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); (_a < _b) ? _a : _b; })

#endif // UTIL_H