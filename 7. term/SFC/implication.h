/***************************************************************************
**           Author: Ondrej Vales                                         **
**            Email: xvales03@stud.fit.vutbr.cz                           **
**             Date: 27. 10. 2017                                         **
**          Project: SFC Demonstration of fuzzy implications              **
****************************************************************************/

#ifndef IMPLICATION_H
#define IMPLICATION_H

#include <QWidget>
#include <QLabel>

#include"resources/qcustomplot.h"

double Godel(double x, double y);
double KleeneDienes(double x, double y);
double KleeneDienesLukasievicz(double x, double y);
double StandardStrict(double x, double y);
double Larsen(double x, double y);
double Mamdani(double x, double y);

class Overlay : public QWidget
{
public:
    Overlay(QWidget *parent);
    void SetCoordX(int x);
    void SetCoordY(int y);
private:
    void paintEvent(QPaintEvent* event);
    int xCoord, yCoord;
};

class Implication : public QWidget
{
public:
    Implication(double (*implication)(double, double), int xCoord, int yCoord, QString titleString, QString equationString, QWidget *parent);
    ~Implication();
    void UpdateX(int x);
    void UpdateY(int y);

private:
    void initElements(int xCoord, int yCoord);
    QCPColorMap* initColorMap();
    QCPColorScale* initColorScale();
    void redraw();
    int x, y;
    QWidget* parent;
    QCPColorMap* initMap(double (*implication)(double, double));
    QCPColorScale* initScale();
    double (*implication)(double, double);
    QCustomPlot *plot;
    QLabel *title, *equation ,*result;
    Overlay *overlay;
};

#endif // IMPLICATION_H
