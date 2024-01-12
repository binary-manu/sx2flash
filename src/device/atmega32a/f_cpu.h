#ifndef F_CPU
#  define F_CPU 1000000
#  warning F_CPU not defined: assuming 1MHz CPU clock
#elif F_CPU <= 0
#  error F_CPU must be positive
#endif