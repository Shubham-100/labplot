/***************************************************************************
    File                 : nsl_diff.c
    Project              : LabPlot
    Description          : NSL numerical differentiation functions
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Stefan Gerlach (stefan.gerlach@uni.kn)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

/* TODO:
	* check that all functions really work with minimum number of points
	* add more orders
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include "nsl_diff.h"
#include "nsl_sf_poly.h"

double nsl_diff_first_central(double xm, double fm, double xp, double fp) {
	return (fp - fm)/(xp - xm);
}

int nsl_diff_deriv_first_equal(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy, oldy, oldy2;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0)	/* forward */
			dy = (-y[2] + 4.*y[1] - 3.*y[0])/(x[2]-x[0]);
		else if (i == n-1) {	/* backward */
			y[i] = (3.*y[i] - 4.*y[i-1] + y[i-2])/(x[i]-x[i-2]);
			y[i-1] = oldy;
		}
		else
			dy = (y[i+1]-y[i-1])/(x[i+1]-x[i-1]);

		if (i > 1)
			y[i-2] = oldy2;
		if (i > 0 && i < n-1)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 2:
		return nsl_diff_first_deriv_second_order(x, y, n);
	case 4:
		return nsl_diff_first_deriv_fourth_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_first_deriv() unsupported order %d\n", order);
		return -1;
	}

	return 0;
}

int nsl_diff_first_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy, oldy, oldy2, xdata[3], ydata[3];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0) {
			/* 3-point forward */
			for (j=0; j < 3; j++)
				xdata[j]=x[j], ydata[j]=y[j];
			dy = nsl_sf_poly_interp_lagrange_2_deriv(x[0], xdata, ydata);
		} else if (i == n-1) {
			/* 3-point backward */
			y[i] = nsl_sf_poly_interp_lagrange_2_deriv(x[i], xdata, ydata);
			y[i-1] = oldy;
		} else {
			/* 3-point center */
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];
			dy = nsl_sf_poly_interp_lagrange_2_deriv(x[i], xdata, ydata);
		}

		if (i > 1)
			y[i-2] = oldy2;
		if (i > 0 && i < n-1)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_first_deriv_fourth_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	double dy[5], xdata[5], ydata[5];
	size_t i, j;
	for (i=0; i < n; i++) {
		if (i == 0)
			for (j=0; j < 5; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i>1 && i<n-2)
			for (j=0; j < 5; j++)
				xdata[j]=x[i-2+j], ydata[j]=y[i-2+j];

		dy[0] = nsl_sf_poly_interp_lagrange_4_deriv(x[i], xdata, ydata);

		if (i == n-1)
			for (j=0; j < 4; j++)
				y[i-j] = dy[j];

		if (i > 3)
			y[i-4] = dy[4];
		for (j=4; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_first_deriv_avg(const double *x, double *y, const size_t n) {
	if (n < 1)
		return -1;

	size_t i;
	double dy, oldy;
	for (i=0; i<n; i++) {
		if(i == 0)
			dy = (y[1]-y[0])/(x[1]-x[0]);
		else if (i == n-1)
			y[i] = (y[i]-y[i-1])/(x[i]-x[i-1]);
		else
			dy = ( (y[i+1]-y[i])/(x[i+1]-x[i]) + (y[i]-y[i-1])/(x[i]-x[i-1]) )/2.;

		if (i > 0)
			y[i-1] = oldy;

		oldy = dy;
	}

	return 0;
}

int nsl_diff_second_deriv(const double *x, double *y, const size_t n, int order) {
	switch (order) {
	case 1:
		return nsl_diff_second_deriv_first_order(x, y, n);
	case 2:
		return nsl_diff_second_deriv_second_order(x, y, n);
	case 3:
		return nsl_diff_second_deriv_third_order(x, y, n);
	/*TODO: higher order */
	default:
		printf("nsl_diff_second_deriv() unsupported order %d\n", order);
		return -1;
	}

	return 0;
}

int nsl_diff_second_deriv_first_order(const double *x, double *y, const size_t n) {
	if (n < 3)
		return -1;

	double dy[3], xdata[3], ydata[3];
	size_t i, j;
	for (i=0; i<n; i++) {
		if (i == 0)
			for (j=0; j < 3; j++)
				xdata[j]=x[j], ydata[j]=y[j];
		else if (i > 0 && i < n-1)
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];

		dy[0] = nsl_sf_poly_interp_lagrange_2_deriv2(xdata, ydata);

		if (i == n-1) {
			y[i] = dy[0];
			y[i-1] = dy[1];
		}

		if (i > 1)
			y[i-2] = dy[2];
		if (i > 0)
			dy[2] = dy[1];

		dy[1] = dy[0];
	}

	return 0;
}

int nsl_diff_second_deriv_second_order(const double *x, double *y, const size_t n) {
	if (n < 4)
		return -1;

	double dy[4], xdata[4], ydata[4];
	size_t i, j;
	for (i=0; i<n; i++) {
		if (i == 0) {
			/* 4-point forward */
			for (j=0; j < 4; j++)
				xdata[j]=x[j], ydata[j]=y[j];
			dy[0] = nsl_sf_poly_interp_lagrange_3_deriv2(x[i], xdata, ydata);
		}
		else if (i == n-1) {
			/* 4-point backward */
			for (j=0; j < 4; j++)
				xdata[j]=x[i-3+j], ydata[j]=y[i-3+j];
			y[i] = nsl_sf_poly_interp_lagrange_3_deriv2(x[i], xdata, ydata);
			y[i-1] = dy[1];
			y[i-2] = dy[2];
		}
		else {
			/* 3-point center */
			for (j=0; j < 3; j++)
				xdata[j]=x[i-1+j], ydata[j]=y[i-1+j];
			dy[0] = nsl_sf_poly_interp_lagrange_2_deriv2(xdata, ydata);
		}

		if (i > 2)
			y[i-3] = dy[3];
		for (j=3; j > 0; j--)
			if (i >= j-1)
				dy[j] = dy[j-1];
	}

	return 0;
}

int nsl_diff_second_deriv_third_order(const double *x, double *y, const size_t n) {
	if (n < 5)
		return -1;

	/*TODO: use interp. poly. */
	double h1, h2, h3, h4, h12, h23, h34, h13, h24, h14, dy, oldy, oldy2, oldy3, oldy4;
	size_t i;
	for (i=0; i < n; i++) {
		if (i == 0) {
			h1 = x[1]-x[0];
			h2 = x[2]-x[1];
			h3 = x[3]-x[2];
			h4 = x[4]-x[3];
			h12 = h1 + h2;
			h23 = h2 + h3;
			h34 = h3 + h4;
			h13 = h12 + h3;
			h24 = h23 + h4;
			h14 = h12 + h34;
			/* 5-point forward */
			dy = 2.*( y[0]*(6.*h1*h1+3.*h2*h2+h3*h34+2.*h2*(h3+h34)+3.*h1*(h2+h23+h24))/(h1*h12*h13*h14) 
					- y[1]*(3.*h1*h1+3.*h2*h2+h3*h34+2.*h2*(h3+h34)+2.*h1*(h2+h23+h24))/(h1*h2*h23*h24)
					+ y[2]*(3.*h1*h1+h23*h24+2.*h1*(h23+h24))/(h12*h2*h3*h34)
					- y[3]*(3.*h1*h1+h2*h24+2.*h1*(h2+h24))/(h13*h23*h3*h4)
					+ y[4]*(3.*h1*h1+h2*h23+2.*h1*(h2+h23))/(h14*h24*h34*h4) );
		} else if (i == 1) {
			/* using values from i==0 */
			/* TODO: 5-point left */
			dy = -y[0]*h2*h23*h24/(h1*h12*h13*h14) + y[1]*(1./h1-1./h2-1./h23-1./h24) + y[2]*h1*h23*h24/(h12*h2*h3*h34)
				- y[3]*h1*h2*h24/(h13*h23*h3*h4) + y[4]*h1*h2*h23/(h14*h24*h34*h4);
		} else if (i == n-2) {
			/* using values from i==n-3*/
			/* TODO: 5-point right */
			dy = -y[i-3]*h23*h3*h4/(h1*h12*h13*h14) + y[i-2]*h13*h3*h4/(h1*h2*h23*h24) - y[i-1]*h13*h23*h4/(h12*h2*h3*h34)
				 + y[i]*(1./h13+1./h23+1./h3-1./h4) + y[i+1]*h13*h23*h3/(h14*h24*h34*h4);
		} else if (i == n-1) {
			/* using values from i==n-3*/
			/* TODO: 5-point backward */
			y[i] = y[i-4]*h24*h34*h4/(h1*h12*h13*h14) - y[i-3]*h14*h34*h4/(h1*h2*h23*h24) + y[i-2]*h14*h24*h4/(h12*h2*h3*h34)
				- y[i-1]*h14*h24*h34/(h13*h23*h3*h4) + y[i]*(1./h14+1./h24+1./h34+1./h4);
			y[i-3] = oldy3;
			y[i-2] = oldy2;
			y[i-1] = oldy;
		} else {
			h1 = x[i-1]-x[i-2];
			h2 = x[i]-x[i-1];
			h3 = x[i+1]-x[i];
			h4 = x[i+2]-x[i+1];
			h12 = h1 + h2;
			h23 = h2 + h3;
			h34 = h3 + h4;
			h13 = h12 + h3;
			h24 = h23 + h4;
			h14 = h12 + h34;
			/* 5-point center */
			dy = 2.*( y[i-2]*(h3*h34-h2*(h3+h34))/(h1*h12*h13*h14) - y[i-1]*(h12*(h3+h34)-h3*h34)/(h1*h2*h23*h24)
				+ y[i]*(h12*(h2-2.*h3-h4)+h3*h34-h2*(h3+h34))/(h12*h2*h3*h34)
				+ y[i+1]*(h2*h34+h12*(h34-h2))/(h13*h23*h3*h4) + y[i+2]*(h12*(h2-h3)-h2*h3)/(h14*h24*h34*h4) );
		}

		if (i > 3)
			y[i-4] = oldy4;
		if (i > 2)
			oldy4 = oldy3;
		if (i > 1)
			oldy3 = oldy2;
		if (i > 0)
			oldy2 = oldy;

		oldy = dy;
	}

	return 0;
}
