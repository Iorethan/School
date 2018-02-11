/***************************************************************************
**           Author: Ondrej Vales                                         **
**            Email: xvales03@stud.fit.vutbr.cz                           **
**             Date: 27. 10. 2017                                         **
**          Project: SFC Demonstration of fuzzy implications              **
****************************************************************************/

#include "implication.h"

double Godel(double x, double y)
{
    return x <= y ? 1 : y;
}

double KleeneDienes(double x, double y)
{
    return (1 - x) > y ? 1 - x : y;
}

double KleeneDienesLukasievicz(double x, double y)
{
    return 1 - x + x *y;
}

double StandardStrict(double x, double y)
{
    return x <= y ? 1 : 0;
}

double Larsen(double x, double y)
{
    return x * y;
}

double Mamdani(double x, double y)
{
    return x <= y ? x : y;
}

Overlay::Overlay(QWidget *parent): QWidget{parent}
{
}

void Overlay::SetCoordX(int x)
{
    xCoord = x;
}

void Overlay::SetCoordY(int y)
{
    yCoord = y;
}

void Overlay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPen pen(Qt::darkGreen);
    pen.setWidth(2);
    painter.setPen(pen);
    QRect rect(50 + xCoord * 1.52, 195 - yCoord * 1.44, 8, 8);
    painter.drawEllipse(rect);
}

Implication::Implication(double (*implication)(double, double), int xCoord, int yCoord, QString titleString, QString equationString, QWidget *parent):
    parent(parent),
    implication(implication),
    plot(new QCustomPlot(parent)),
    title(new QLabel(titleString, parent)),
    equation(new QLabel(equationString, parent)),
    result(new QLabel(parent)),
    overlay(new Overlay(parent))
{
    initElements(xCoord, yCoord);

    QCPColorMap *colorMap = initColorMap();
    QCPColorScale *colorScale = initColorScale();
    plot->plotLayout()->addElement(0, 1, colorScale);

    colorMap->setColorScale(colorScale);
    colorMap->setGradient(QCPColorGradient::gpPolar);
    colorMap->rescaleDataRange();

    QCPMarginGroup *marginGroup = new QCPMarginGroup(plot);
    plot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    redraw();
}

Implication::~Implication()
{
    delete plot;
    delete title;
    delete equation;
    delete result;
    delete overlay;
}


void Implication::initElements(int xCoord, int yCoord)
{
    title->setGeometry(xCoord, yCoord, 300, 20);
    equation->setGeometry(xCoord, yCoord + 20, 300, 20);
    plot->setGeometry(xCoord, yCoord + 40, 300, 200);
    result->setGeometry(xCoord, yCoord + 240, 300, 20);
    overlay->setGeometry(xCoord, yCoord, 300, 260);

    plot->xAxis->setLabel("x");
    plot->yAxis->setLabel("y");
    plot->xAxis->setRange(0, 1);
    plot->yAxis->setRange(0, 1);
    plot->setBackground(parent->QWidget::palette().background().color());
}

QCPColorMap* Implication::initColorMap()
{
    QCPColorMap *colorMap = new QCPColorMap(plot->xAxis, plot->yAxis);
    int xSize = 101;
    int ySize = 101;
    colorMap->data()->setSize(xSize, ySize);
    colorMap->data()->setRange(QCPRange(0, 1), QCPRange(0, 1));



    QCPItemEllipse *selection = new QCPItemEllipse(plot);
    selection->setPen(QPen(Qt::black));
    selection->setBrush(QBrush(Qt::SolidLine));
    selection->topLeft->setCoords(0, 0);
    selection->bottomRight->setCoords(100, 100);

    double xVal, yVal;
    for (int xIndex = 0; xIndex < xSize; ++xIndex)
    {
      for (int yIndex = 0; yIndex < ySize; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &xVal, &yVal);
        colorMap->data()->setCell(xIndex, yIndex, implication(xVal, yVal));
      }
    }
    return colorMap;
}

QCPColorScale* Implication::initColorScale()
{
    QCPColorScale *colorScale = new QCPColorScale(plot);
    colorScale->setType(QCPAxis::atRight);
    colorScale->axis()->setLabel("Implication value");
    return colorScale;
}

void Implication::UpdateX(int x)
{
    this->x = x;
    overlay->SetCoordX(x);
    redraw();
}

void Implication::UpdateY(int y)
{
    this->y = y;
    overlay->SetCoordY(y);
    redraw();
}

void Implication::redraw()
{
    result->setText(QString{ "m(x -> y) = %1" }.arg( implication(x / 100.0, y / 100.0), 3, 'f', 2, '0' ));
    plot->replot();
    overlay->repaint();
}
