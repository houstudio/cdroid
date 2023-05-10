#include <cdroid.h>
#include <widget/plotaxis.h>
#include <widget/plotobject.h>
#include <widget/plotview.h>

void slotSelectPlot(PlotView*plot,int n);
int main(int argc,char*srgv[]){
   App app;
   Window*w=new Window(0,0,-1,-1);
   PlotView*plot=new PlotView(400,400);
   w->addView(plot);
   plot->setShowGrid(true);
   
   plot->invalidate();
   slotSelectPlot(plot,argc-1);   
   return app.exec(); 
}

void slotSelectPlot(PlotView*plot,int n)
{
    plot->resetPlot();
    PlotObject* po1;
    PlotObject* po2;
    switch (n) {
    case 0: { // Points plot
        plot->setLimits(-6.0, 11.0, -10.0, 110.0);

        po1 = new PlotObject(0xFFFFFFFF, PlotObject::Points, 4, PlotObject::Asterisk);
        po2 = new PlotObject(0xFF00FF00, PlotObject::Points, 4, PlotObject::Triangle);

        for (float x = -5.0; x <= 10.0; x += 1.0) {
            po1->addPoint(x, x * x);
            po2->addPoint(x, 50.0 - 5.0 * x);
        }

        plot->addPlotObject(po1);
        plot->addPlotObject(po2);

        plot->invalidate();
        break;
    }

    case 1: { // Lines plot
        plot->setLimits(-0.1, 6.38, -1.1, 1.1);
        plot->setSecondaryLimits(-5.73, 365.55, -1.1, 1.1);
        plot->axis(PlotView::TopAxis)->setTickLabelsShown(true);
        plot->axis(PlotView::BottomAxis)->setLabel(std::string("Angle [radians]"));
        plot->axis(PlotView::TopAxis)->setLabel(std::string("Angle [degrees]"));

        po1 = new PlotObject(0xFFFF0000, PlotObject::Lines, 2);
        po2 = new PlotObject(0xFFAABBCC, PlotObject::Lines, 2);

        for (float t = 0.0; t <= 6.28; t += 0.04) {
            po1->addPoint(t, sin(t));
            po2->addPoint(t, cos(t));
        }

        plot->addPlotObject(po1);
        plot->addPlotObject(po2);

        plot->invalidate();
        break;
    }

    case 2: { // Bars plot
        plot->setLimits(-7.0, 7.0, -5.0, 105.0);

        po1 = new PlotObject(0xFFFFFFFF, PlotObject::Bars, 2);
        po1->setBarBrush(0xFF00FF00);//QBrush(Qt::green, Qt::Dense4Pattern));

        for (float x = -6.5; x <= 6.5; x += 0.5) {
            po1->addPoint(x, 100 * exp(-0.5 * x * x), std::string(""), 0.5);
        }

        plot->addPlotObject(po1);

        plot->invalidate();
        break;
    }

    case 3: { // Points plot with labels
        plot->setLimits(-1.1, 1.1, -1.1, 1.1);

        po1 = new PlotObject(Color::YELLOW, PlotObject::Points, 10, PlotObject::Star);
        po1->setLabelPen(0xFF00FF00);//QPen(Qt::green));

        po1->addPoint(0.0, 0.8, std::string("North"));
        po1->addPoint(0.57, 0.57, std::string("Northeast"));
        po1->addPoint(0.8, 0.0, std::string("East"));
        po1->addPoint(0.57, -0.57, std::string("Southeast"));
        po1->addPoint(0.0, -0.8, std::string("South"));
        po1->addPoint(-0.57, -0.57, std::string("Southwest"));
        po1->addPoint(-0.8, 0.0, std::string("West"));
        po1->addPoint(-0.57, 0.57, std::string("Northwest"));

        plot->addPlotObject(po1);

        plot->invalidate();
        break;
    }

    case 4: { // Points, Lines and Bars plot
        plot->setLimits(-2.1, 2.1, -0.1, 4.1);

        po1 = new PlotObject(Color::WHITE, PlotObject::Points, 10, PlotObject::Pentagon);

        po1->setShowLines(true);
        po1->setShowBars(true);
        po1->setLabelPen(0xFFAA8800);//QPen(QColor("#AA8800")));
        po1->setLinePen(0xFFFF0000);//QPen(Qt::red, 3.0, Qt::DashDotLine));
        po1->setBarBrush(0xFF0000FF);//QBrush(Qt::blue, Qt::BDiagPattern));

        po1->addPoint(-1.75, 0.5);
        po1->addPoint(-1.25, 1.0);
        po1->addPoint(-0.75, 1.25);
        po1->addPoint(-0.25, 1.5);
        po1->addPoint(0.25, 2.5);
        po1->addPoint(0.75, 3.0);
        po1->addPoint(1.25, 1.5);
        po1->addPoint(1.75, 1.75);

        plot->addPlotObject(po1);

        plot->invalidate();
        break;
    }

    case 5: { // Points, Lines and Bars plot with labels
        plot->setLimits(-2.1, 2.1, -0.1, 4.1);

        po1 = new PlotObject(0xFFFFFFFF, PlotObject::Points, 10, PlotObject::Pentagon);

        po1->setShowLines(true);
        po1->setShowBars(true);
        po1->setLabelPen(0xFFAA8800);
        po1->setLinePen(0xFFFF0000);//, 3.0, Qt::DashDotLine));
        po1->setBarBrush(0xFF0000FF);//QBrush(Qt::blue, Qt::BDiagPattern));

        po1->addPoint(-1.75, 0.5, std::string("A"));
        po1->addPoint(-1.25, 1.0, std::string("B"));
        po1->addPoint(-0.75, 1.25, std::string("C"));
        po1->addPoint(-0.25, 1.5, std::string("D"));
        po1->addPoint(0.25, 2.5, std::string("E"));
        po1->addPoint(0.75, 3.0, std::string("F"));
        po1->addPoint(1.25, 1.5, std::string("G"));
        po1->addPoint(1.75, 1.75, std::string("H"));

        plot->addPlotObject(po1);

        plot->invalidate();
        break;
    }
    }
}
