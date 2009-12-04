/********************************************************************
* Description: gib Norbert's Simulationsdaten aus: 10 Bit Dreieckssignal
* Author: Norbert Flatinger, norbert.flatinger@kit.edu
* Created at: Fri Dec  4 10:46:44 CET 2009
* Computer: ipeflatinger1
* System: Linux 2.6.26-2-686 on i686
*
* Copyright (c) 2009 Norbert Flatinger  All rights reserved.
*
********************************************************************/

#include <iostream>
#include <iomanip>

using namespace std;

int main(){
	int i;			// data value
	int m;			// minute
	int s;			// second
	bool counting_up;

	// set start values
	i = 0;
	m = 3;	// this must be low, so that it does not exceed 59 later on!!!
	s = 39;
	counting_up = true;

	// output file "header"
	cout << "# Norbert's Simulations Device" << endl;
	cout << "# Datum:\t" << "03.12.2009" << endl;

	for (m = m; m < 60; m++) {
		for (s = s; s < 60; s++) {
			// output "timestamp" and "data value"
			cout << "11:" << setfill('0') << setw(2) << m << ":" << setw(2)
				 << s << '\t' << setfill(' ') << setw(4) << i << endl;
/*			cout << "11:";
			cout.fill('0');
			cout.width(2);
			cout << m << ":";
			cout.width(2);
			cout << s << '\t';
			cout.fill(' ');
			cout.width(4);
			cout << i << endl;
*/
			if (counting_up) {
				// increment data value
				i++;
				// allow max. 1023 (= 10 bits) for data value, then go downwards again
				if (i == 1024 ) {
					i = 1022;
					counting_up = false;
				}
			} else {
				// decrement data value
				i--;
				// finished, when back to 0 again
				if (i == 0 ) break;
			}
		}
		// reset second variable
		s = 0;

		// finished, when back to 0 again
		if (i == 0 ) break;
	}
}
