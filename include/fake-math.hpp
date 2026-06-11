#ifndef FAKEMATH_HPP
#define FAKEMATH_HPP

#include <stddef.h>

class Base {
public:
	Base(int param1, int param2);
	Base(int param1);

	void show_params();
	int get_param(char *param_name);

private:
	int parameter1;
	int parameter2;
};

#endif // FAKEMATH_HPP