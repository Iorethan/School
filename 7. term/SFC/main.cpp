/***************************************************************************
**           Author: Ondrej Vales                                         **
**            Email: xvales03@stud.fit.vutbr.cz                           **
**             Date: 27. 10. 2017                                         **
**          Project: SFC Demonstration of fuzzy implications              **
****************************************************************************/

#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Window w;
    w.showMaximized();

    return a.exec();
}
