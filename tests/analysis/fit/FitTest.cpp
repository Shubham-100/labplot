/***************************************************************************
    File                 : FitTest.cpp
    Project              : LabPlot
    Description          : Tests for data fitting
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2018 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "FitTest.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}

void FitTest::initTestCase() {
	// needed in order to have the signals triggered by SignallingUndoCommand, see LabPlot.cpp
	//TODO: redesign/remove this
	qRegisterMetaType<const AbstractAspect*>("const AbstractAspect*");
	qRegisterMetaType<const AbstractColumn*>("const AbstractColumn*");
}

//##############################################################################
//#################  linear regression with NIST datasets ######################
//##############################################################################
void FitTest::testLinearNorris() {
	//NIST data for Norris dataset
	QVector<double> xData = {0.2,337.4,118.2,884.6,10.1,226.5,666.3,996.3,448.6,777.0,558.2,0.4,0.6,775.5,666.9,338.0,447.5,11.6,556.0,228.1,
		995.8,887.6,120.2,0.3,0.3,556.8,339.1,887.2,999.0,779.0,11.1,118.3,229.2,669.1,448.9,0.5};
	QVector<double> yData = {0.1,338.8,118.1,888.0,9.2,228.1,668.5,998.5,449.1,778.9,559.2,0.3,0.1,778.1,668.8,339.3,448.9,10.8,557.7,228.3,
		998.0,888.8,119.6,0.3,0.6,557.6,339.3,888.0,998.5,778.9,10.2,117.6,228.9,668.4,449.2,0.2};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Numeric);
	xDataColumn.replaceValues(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0), -0.262323073774029);
	QCOMPARE(fitResult.errorValues.at(0), 0.232818234301152);
	QCOMPARE(fitResult.paramValues.at(1), 1.00211681802045);
	QCOMPARE(fitResult.errorValues.at(1), 0.429796848199937e-3);

	QCOMPARE(fitResult.rsd, 0.884796396144373);
	QCOMPARE(fitResult.rsquare, 0.999993745883712);
	QCOMPARE(fitResult.sse, 26.6173985294224);
	QCOMPARE(fitResult.rms, 0.782864662630069);
//	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 5436419.54079774
	FuzzyCompare(fitResult.fdist_F, 5436385.54079785, 34.);
}

void FitTest::testLinearPontius() {
	//NIST data for Pontius dataset
	QVector<int> xData = {150000,300000,450000,600000,750000,900000,1050000,1200000,1350000,1500000,1650000,1800000,1950000,2100000,
		2250000,2400000,2550000,2700000,2850000,3000000,150000,300000,450000,600000,750000,900000,1050000,1200000,1350000,1500000,
		1650000,1800000,1950000,2100000,2250000,2400000,2550000,2700000,2850000,3000000};
	QVector<double> yData = {.11019,.21956,.32949,.43899,.54803,.65694,.76562,.87487,.98292,1.09146,1.20001,1.30822,1.41599,1.52399,
		1.63194,1.73947,1.84646,1.95392,2.06128,2.16844,.11052,.22018,.32939,.43886,.54798,.65739,.76596,.87474,
		.98300,1.09150,1.20004,1.30818,1.41613,1.52408,1.63159,1.73965,1.84696,1.95445,2.06177,2.16829};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 2;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 3);

	QCOMPARE(fitResult.paramValues.at(0), 0.673565789473684e-3);
	QCOMPARE(fitResult.errorValues.at(0), 0.107938612033077e-3);
	QCOMPARE(fitResult.paramValues.at(1), 0.732059160401003e-6);
	QCOMPARE(fitResult.errorValues.at(1), 0.157817399981659e-9);
	QCOMPARE(fitResult.paramValues.at(2), -0.316081871345029e-14);
	QCOMPARE(fitResult.errorValues.at(2), 0.486652849992036e-16);

	QCOMPARE(fitResult.rsd, 0.205177424076185e-3);
	QCOMPARE(fitResult.rsquare, 0.999999900178537);
	QCOMPARE(fitResult.sse, 0.155761768796992e-5);
	QCOMPARE(fitResult.rms, 0.420977753505385e-7);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 370661768.991551
	//FuzzyCompare(fitResult.fdist_F, 185330865.995752, 1.);
}

void FitTest::testLinearNoInt1() {
	//NIST data for NoInt1 dataset
	QVector<int> xData = {60,61,62,63,64,65,66,67,68,69,70};
	QVector<int> yData = {130,131,132,133,134,135,136,137,138,139,140};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_custom;
	XYFitCurve::initFitData(fitData);
	fitData.model = "c * x";
	fitData.paramNames << "c";
	const int np = fitData.paramNames.size();
	fitData.paramStartValues << 1.;
	fitData.paramLowerLimits << -std::numeric_limits<double>::max();
	fitData.paramUpperLimits << std::numeric_limits<double>::max();
	//fitData.eps = 1.e-15;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	QCOMPARE(np, 1);

	DEBUG(std::setprecision(15) << fitResult.paramValues.at(0));	// result: 2.07438016513166
	FuzzyCompare(fitResult.paramValues.at(0), 2.07438016528926, 1.e-9);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(0));	// result: 0.00463315628245255
//	QCOMPARE(fitResult.errorValues.at(0), 0.165289256198347e-1);

	QCOMPARE(fitResult.rsd, 3.56753034006338);
//	QCOMPARE(fitResult.rsquare, 0.999365492298663);
	QCOMPARE(fitResult.sse, 127.272727272727);
	QCOMPARE(fitResult.rms, 12.7272727272727);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 370661768.991551
//	FuzzyCompare(fitResult.fdist_F, 15750.25, 1.);
}
void FitTest::testLinearNoInt1_2() {
	//NIST data for NoInt1 dataset
	QVector<int> xData = {60,61,62,63,64,65,66,67,68,69,70};
	QVector<int> yData = {130,131,132,133,134,135,136,137,138,139,140};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 1;
	XYFitCurve::initFitData(fitData);
	fitData.paramStartValues[0] = 0;
	fitData.paramFixed[0] = true;
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 2);

	QCOMPARE(fitResult.paramValues.at(0), 0.);
	QCOMPARE(fitResult.paramValues.at(1), 2.07438016528926);
	DEBUG(std::setprecision(15) << fitResult.errorValues.at(1));	// result: 0.0046331562857966
	//QCOMPARE(fitResult.errorValues.at(1), 0.165289256198347e-1);

//	QCOMPARE(fitResult.rsd, 3.56753034006338);
//	QCOMPARE(fitResult.rsquare, 0.999365492298663);
	QCOMPARE(fitResult.sse, 127.272727272727);
//	QCOMPARE(fitResult.rms, 12.7272727272727);
	DEBUG(std::setprecision(15) << fitResult.fdist_F);	// result: 7.77857142857144
//	FuzzyCompare(fitResult.fdist_F, 15750.25, 1.);
}

void FitTest::testLinearWampler1() {
	// TODO: remove when finished
//	qFatal("STOP!");
	//NIST data for Wampler1 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {1,6,63,364,1365,3906,9331,19608,37449,66430,111111,
		177156,271453,402234,579195,813616,1118481,1508598,2000719,2613660,3368421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 0.0);
	QCOMPARE(fitResult.rsquare,  1.0);
	QCOMPARE(fitResult.sse, 0.0);
	QCOMPARE(fitResult.rms, 0.0);
}

void FitTest::testLinearWampler2() {
	//NIST data for Wampler2 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {1.00000,1.11111,1.24992,1.42753,1.65984,1.96875,2.38336,2.94117,3.68928,4.68559,
		6.00000,7.71561,9.92992,12.75603,16.32384,20.78125,26.29536,33.05367,41.26528,51.16209,63.00000};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	qDebug()<<"STATUS " << fitResult.status;
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	QCOMPARE(fitResult.paramValues.at(0), 1.0);
	QCOMPARE(fitResult.paramValues.at(1), 0.1);
	QCOMPARE(fitResult.paramValues.at(2), 0.01);
	QCOMPARE(fitResult.paramValues.at(3), 0.001);
	QCOMPARE(fitResult.paramValues.at(4), 0.0001);
	QCOMPARE(fitResult.paramValues.at(5), 0.00001);

		//TODO: rsd, sse and rms fails with
	//FAIL!  : FitTest::testLinearWampler2() Compared doubles are not the same (fuzzy compare)
	//Actual   (fitResult.rsd): 2,32459e-15
	//Expected (0.0)          : 0
	//etc.
// 	QCOMPARE(fitResult.rsd, 0.000000000000000);
	QCOMPARE(fitResult.rsquare,  1.00000000000000);
// 	QCOMPARE(fitResult.sse, 0.000000000000000);
// 	QCOMPARE(fitResult.rms, 0.000000000000000);
}

void FitTest::testLinearWampler3() {
	//NIST data for Wampler3 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<double> yData = {760.,-2042.,2111.,-1684.,3888.,1858.,11379.,17560.,39287.,64382.,113159.,
		175108.,273291.,400186.,581243.,811568.,1121004.,1506550.,2002767.,2611612.,3369180.};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Numeric);
	yDataColumn.replaceValues(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 2360.14502379268);
	QCOMPARE(fitResult.rsquare,  0.999995559025820);
	QCOMPARE(fitResult.sse,  83554268.0000000);
	QCOMPARE(fitResult.rms, 5570284.53333333);
}

void FitTest::testLinearWampler4() {
	//NIST data for Wampler4 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {75901,-204794,204863,-204436,253665,-200894,214131,-185192,221249,-138370,
		315911,-27644,455253,197434,783995,608816,1370781,1303798,2205519,2408860,3444321};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 236014.502379268);
	QCOMPARE(fitResult.rsquare,  0.957478440825662);
	QCOMPARE(fitResult.sse, 835542680000.000);
	QCOMPARE(fitResult.rms, 55702845333.3333);
}

void FitTest::testLinearWampler5() {
	//NIST data for Wampler5 dataset
	QVector<int> xData = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	QVector<int> yData = {7590001,-20479994,20480063,-20479636,25231365,-20476094,20489331,-20460392,18417449,-20413570,
				20591111,-20302844,18651453,-20077766,21059195,-19666384,26348481,-18971402,22480719,-17866340,10958421};

	//data source columns
	Column xDataColumn("x", AbstractColumn::Integer);
	xDataColumn.replaceInteger(0, xData);

	Column yDataColumn("y", AbstractColumn::Integer);
	yDataColumn.replaceInteger(0, yData);

	XYFitCurve fitCurve("fit");
	fitCurve.setXDataColumn(&xDataColumn);
	fitCurve.setYDataColumn(&yDataColumn);

	//prepare the fit
	XYFitCurve::FitData fitData = fitCurve.fitData();
	fitData.modelCategory = nsl_fit_model_basic;
	fitData.modelType = nsl_fit_model_polynomial;
	fitData.degree = 5;
	XYFitCurve::initFitData(fitData);
	fitCurve.setFitData(fitData);

	//perform the fit
	fitCurve.recalculate();
	const XYFitCurve::FitResult& fitResult = fitCurve.fitResult();

	//check the results
	QCOMPARE(fitResult.available, true);
	QCOMPARE(fitResult.valid, true);

	const int np = fitData.paramNames.size();
	QCOMPARE(np, 6);

	for (int i = 0; i < np; i++) {
		const double paramValue = fitResult.paramValues.at(i);
		QCOMPARE(paramValue, 1.0);
	}

	QCOMPARE(fitResult.rsd, 23601450.2379268);
	QCOMPARE(fitResult.rsquare, 0.224668921574940E-02);
	QCOMPARE(fitResult.sse, 0.835542680000000E+16);
	QCOMPARE(fitResult.rms, 557028453333333.);
}

//##############################################################################
//#############  non-linear regression with NIST datasets  #####################
//##############################################################################

//TODO

//##############################################################################
//#########################  Fits with weights #################################
//##############################################################################

//TODO

QTEST_MAIN(FitTest)
