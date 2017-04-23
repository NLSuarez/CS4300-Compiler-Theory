#include <iostream>

/* 

	This is a block comment			

*/

//	This is a line comment

int main()
{

/*

        This is a block comment

*/

    int x = 5;		// This is a line comment
    int y = 10;
    float pi_3 = 3.14;

    cout << pi_3 + x << endl;

    cin >> x;
    x += 1;
    x -= 5;

    cout << x * 10 << " " << x / y << " " << y % x << endl;

    cout << "This is a literal string" << endl;

    if (!(x >= 5) && y < 10 || x == y || pi_3 != 3.14 || x > 3 || y <= 125)
        cout << "Whatever" << endl;
    else
	cout << "Also whatever" << endl;

    return 0;
}

