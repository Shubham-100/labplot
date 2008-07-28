#ifndef PLOT2DSIMPLE_H
#define PLOT2DSIMPLE_H

#include "Plot2D.h"

/**
 * @brief Inherits Plot2D, implements the function drawCurves()
 * to draw the points (function values) of a 2D-function.
 */
class Plot2DSimple:public Plot2D {

public:
	Plot2DSimple();
	void calculateXY(Point d,double *x, double *y, int w, int h);
	void drawCurves(QPainter *p, int w, int h);
};

#endif // PLOT2DSIMPLE_H
