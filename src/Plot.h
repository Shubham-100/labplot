//LabPlot : Plot.h

#ifndef PLOT_H
#define PLOT_H

#include <QBrush>
#include "elements/Point.h"
#include "elements/Range.h"
#include "elements/Label.h"
#include "plottype.h"

class Set;
class Axis;
class Style;
class Symbol;

class Plot {
public:
	Plot();
	virtual ~Plot() {}
	PlotType plotType() const { return m_plotType; }
	Label *Title() const { return title; }
	void addSet(Set *s) { set.append(s); }
	virtual void setRange(Range *,int i) = 0;
	Range* getRange(int i) { return &range[i];}
	virtual void setActRange(Range *,int i) = 0;
	Range* ActRange(int i) { return &actrange[i];}
	virtual void setRanges(Range *) = 0;
	Range* Ranges() { return range;}
	virtual void setActRanges(Range *) = 0;
	Range* ActRanges() { return actrange;}
	virtual void draw(QPainter *p, const int w, const int h) const = 0 ;
	QString TicLabel(int atlf, int prec, QString dtf, double value) const;
	void resetRanges();
	void drawStyle(QPainter *p, Style *style, Symbol *symbol, QVector<QPoint> pa, int xmin, int xmax, int ymin, int ymax);
protected:
	PlotType m_plotType;
	QList<Set *> set;			//!< data set list
	QList<Axis *> axis;			//!< axis list
	Point p1, p2;				//!< plotting area	(0..1)
	Label *title;
	Range range[3];				//!< ranges of set (x,y,z)
	Range actrange[3];			//!< actual plotting range (x,y,z)
	QBrush background, areabackground;
	Point position, size;
	bool transparent;
};

#endif // PLOT_H
