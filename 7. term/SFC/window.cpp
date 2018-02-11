/***************************************************************************
**           Author: Ondrej Vales                                         **
**            Email: xvales03@stud.fit.vutbr.cz                           **
**             Date: 27. 10. 2017                                         **
**          Project: SFC Demonstration of fuzzy implications              **
****************************************************************************/

#include "window.h"
#include "ui_window.h"
#include "implication.h"

Window::Window(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Window),
    xslider(new QSlider(Qt::Horizontal, this)),
    yslider(new QSlider(Qt::Horizontal, this)),
    xlabel(new QLabel(this)),
    ylabel(new QLabel(this))
{
    ui->setupUi(this);
    initImplications();
    initSlider(xslider, 10, 10, 100, 20);
    initSlider(yslider, 10, 30, 100, 20);

    xlabel->setGeometry(120, 10, 100, 20);
    ylabel->setGeometry(120, 30, 100, 20);
    SetYValue(0);
    SetXValue(0);

    connect(xslider, SIGNAL(valueChanged(int)), this, SLOT(SetXValue(int)));
    connect(yslider, SIGNAL(valueChanged(int)), this, SLOT(SetYValue(int)));
}

Window::~Window()
{
    delete xlabel;
    delete ylabel;
    delete xslider;
    delete yslider;
    foreach (auto item, implications)
    {
        delete item;
    }
    delete ui;
}

void Window::SetXValue(int value)
{
    xlabel->setText(QString{ "m(x) = %1" }.arg( value/100.0, 3, 'f', 2, '0' ));
    foreach (auto item, implications)
    {
        item->UpdateX(value);
    }
}

void Window::SetYValue(int value)
{
    ylabel->setText(QString{ "m(y) = %1" }.arg( value/100.0, 3, 'f', 2, '0' ));
    foreach (auto item, implications)
    {
        item->UpdateY(value);
    }
}

void Window::initImplications()
{
    implications.push_back(new Implication(Godel, 10, 50, QString{"Godel"}, QString{"m(x -> y) = m(x) <= m(y): 1 else: m(y)"}, this));
    implications.push_back(new Implication(KleeneDienes, 320, 50, QString{"Kleene-Dienes"}, QString{"m(x -> y) = max(1 - m(x), m(y))"}, this));
    implications.push_back(new Implication(KleeneDienesLukasievicz, 630, 50, QString{"Kleene-Dienes-Lukasievicz"}, QString{"m(x -> y) = 1 - m(x) + m(x) * m(y))"}, this));
    implications.push_back(new Implication(StandardStrict, 10, 330, QString{"Standard strict"}, QString{"m(x -> y) = m(x) <= m(y): 1 else: 0"}, this));
    implications.push_back(new Implication(Mamdani, 320, 330, QString{"Mamdani (not a classical implication!)"}, QString{"m(x -> y) = min(m(x), m(y))"}, this));
    implications.push_back(new Implication(Larsen, 630, 330, QString{"Larsen (not a classical implication!)"}, QString{"m(x -> y) = m(x) * m(y)"}, this));
}

void Window::initSlider(QSlider *slider, int x, int y, int w, int h)
{
    slider->setGeometry(x, y, w, h);
    slider->setTickPosition(QSlider::NoTicks);
    slider->setValue(0);
    slider->setMinimum(0);
    slider->setMaximum(100);
}
