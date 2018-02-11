/***************************************************************************
**           Author: Ondrej Vales                                         **
**            Email: xvales03@stud.fit.vutbr.cz                           **
**             Date: 27. 10. 2017                                         **
**          Project: SFC Demonstration of fuzzy implications              **
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include<vector>

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPainter>

#include"implication.h"
#include"resources/qcustomplot.h"

using namespace std;

namespace Ui
{
    class Window;
}

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();
    void initImplications();
    void initSlider(QSlider *slider, int x, int y, int w, int h);
    void initPlot(QCustomPlot *plot, int x, int y, int w, int);

public slots:
    void SetXValue(int value);
    void SetYValue(int value);

private:
    Ui::Window *ui;
    QSlider *xslider, *yslider;
    QLabel *xlabel, *ylabel;
    QCustomPlot *plot;
    std::vector<Implication*> implications;
};

#endif // WINDOW_H
