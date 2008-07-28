//LabPlot : Plot2D.h

#ifndef PLOT2D_H
#define PLOT2D_H

#include "Plot.h"

class Plot2D:public Plot {
public:
	// Plot2D(Worksheet *p);
	Plot2D();
	void draw(QPainter *p, const int w, const int h) const;
/*	~Plot2D();
	void saveXML(QDomDocument doc, QDomElement plottag);
	void openXML(QDomElement e);
	void saveAxes(QTextStream *t);
	void openAxes(QTextStream *t, int version);
	Axis *getAxis(int i) { return &axis[i]; }
*/
	void setRanges(Range* r) {range[0]=r[0];range[1]=r[1];}
	void setActRanges(Range* r);
	void setRange(Range* r,int i) {range[i]=*r;}
	void setActRange(Range* r,int i);

	void drawAxes(QPainter *p, const int w, const int h) const;
	void drawAxesTicks(QPainter *p, const int w, const int h, const int k) const;
	void drawBorder(QPainter *p, const int w, const int h) const;
// TODO	virtual void drawCurves(QPainter *p, int w, int h) = 0;
//	virtual void drawFill(QPainter *p, int w, int h) = 0;
};

#endif // PLOT2D_H
