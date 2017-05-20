

#include <iostream>

int x, y, z;

int add(int a, int b) { return a + b; }

int subtract(int a, int b) { return a - b; }

int multiply(int a, int b) { return a * b; }

int divide(int a, int b) { return a / b; }
	
int modulo(int a, int b) { return a % b; }

int main()
{
	x = 8; y = 3;

	cout << "START:  x = 8 | y = 3" << endl << endl;
	
	cout << "z <- add(x, y)" << endl;

	z = add(x, y);

	cout << "z = " << z << endl << endl;

	cout << "z <- subtract(x, y)" << endl;

    z = subtract(x, y);

    cout << "z = " << z << endl << endl;

	cout << "z <- multiply(x, y)" << endl;

    z = multiply(x, y);

    cout << "z = " << z << endl << endl;

	cout << "z <- divide(x, y)" << endl;

    z = divide(x, y);

    cout << "z = " << z << endl << endl;

	cout << "z <- modulo(x, y)" << endl;

    z = modulo(x, y);

    cout << "z = " << z << endl << endl;

	cout << "z <- ((add(x, y) + add(x, z)) * modulo(y, x)" << endl;
	
	z = (add(x, y) + add(x, z)) * modulo(y, x);

	cout << "z = " << z << endl << endl;

	return 0;
}

