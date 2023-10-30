/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 Andreas Nicolai <Andreas.Nicolai@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <widget/plotaxis.h>
#include <math.h> 
#include <iostream>
#include <iomanip>
namespace cdroid{

class PlotAxis::Private
{
public:
    Private(PlotAxis *qq)
        : q(qq)
        , m_visible(true)
        , m_showTickLabels(false)
        , m_labelFmt('g')
        , m_labelFieldWidth(0)
        , m_labelPrec(-1)
    {
    }

    PlotAxis *q;

    bool m_visible : 1; // Property "visible" defines if Axis is drawn or not.
    bool m_showTickLabels : 1;
    bool m_showTickmarks : 1;
    char m_labelFmt; // Number format for number labels, see std::string::arg()
    std::string m_label; // The label of the axis.
    int m_labelFieldWidth; // Field width for number labels, see std::string::arg()
    int m_labelPrec; // Number precision for number labels, see std::string::arg()
    std::list<double> m_MajorTickMarks, m_MinorTickMarks;
};

PlotAxis::PlotAxis(const std::string &label)
    : d(new Private(this))
{
    d->m_label = label;
}

PlotAxis::~PlotAxis()
{
    delete d;
}

bool PlotAxis::isVisible() const
{
    return d->m_visible;
}

void PlotAxis::setVisible(bool visible)
{
    d->m_visible = visible;
}

bool PlotAxis::isTickmarkVisible()const{
    return d->m_showTickmarks;
}

void PlotAxis::setTickmarkVisible(bool visible){
    d->m_showTickmarks = visible;
}

bool PlotAxis::areTickLabelsShown() const
{
    return d->m_showTickLabels;
}

void PlotAxis::setTickLabelsShown(bool b)
{
    d->m_showTickLabels = b;
}

void PlotAxis::setLabel(const std::string &label)
{
    d->m_label = label;
}

std::string PlotAxis::label() const
{
    return d->m_label;
}

void PlotAxis::setTickLabelFormat(char format, int fieldWidth, int precision)
{
    d->m_labelFieldWidth = fieldWidth;
    d->m_labelFmt = format;
    d->m_labelPrec = precision;
}

int PlotAxis::tickLabelWidth() const
{
    return d->m_labelFieldWidth;
}

char PlotAxis::tickLabelFormat() const
{
    return d->m_labelFmt;
}

int PlotAxis::tickLabelPrecision() const
{
    return d->m_labelPrec;
}

void PlotAxis::setTickMarks(double x0, double length)
{
    d->m_MajorTickMarks.clear();
    d->m_MinorTickMarks.clear();

    // s is the power-of-ten factor of length:
    // length = t * s; s = 10^(pwr).  e.g., length=350.0 then t=3.5, s = 100.0; pwr = 2.0
    double pwr = 0.0;
    modf(log10(length), &pwr);
    double s = pow(10.0, pwr);
    double t = length / s;

    double TickDistance = 0.0; // The distance between major tickmarks
    int NumMajorTicks = 0; // will be between 3 and 5
    int NumMinorTicks = 0; // The number of minor ticks between major ticks (will be 4 or 5)

    // adjust s and t such that t is between 3 and 5:
    if (t < 3.0) {
        t *= 10.0;
        s /= 10.0;
        // t is now between 3 and 30
    }

    if (t < 6.0) { // accept current values
        TickDistance = s;
        NumMajorTicks = int(t);
        NumMinorTicks = 5;
    } else if (t < 10.0) { // adjust by a factor of 2
        TickDistance = s * 2.0;
        NumMajorTicks = int(t / 2.0);
        NumMinorTicks = 4;
    } else if (t < 20.0) { // adjust by a factor of 4
        TickDistance = s * 4.0;
        NumMajorTicks = int(t / 4.0);
        NumMinorTicks = 4;
    } else { // adjust by a factor of 5
        TickDistance = s * 5.0;
        NumMajorTicks = int(t / 5.0);
        NumMinorTicks = 5;
    }

    // We have determined the number of tickmarks and their separation
    // Now we determine their positions in the Data space.

    // Tick0 is the position of a "virtual" tickmark; the first major tickmark
    // position beyond the "minimum" edge of the data range.
    double Tick0 = x0 - fmod(x0, TickDistance);
    if (x0 < 0.0) {
        Tick0 -= TickDistance;
        NumMajorTicks++;
    }

    for (int i = 0; i < NumMajorTicks + 2; i++) {
        double xmaj = Tick0 + i * TickDistance;
        if (xmaj >= x0 && xmaj <= x0 + length) {
            d->m_MajorTickMarks.push_back(xmaj);
        }

        for (int j = 1; j < NumMinorTicks; j++) {
            double xmin = xmaj + TickDistance * j / NumMinorTicks;
            if (xmin >= x0 && xmin <= x0 + length) {
                d->m_MinorTickMarks.push_back(xmin);
            }
        }
    }
}

std::string PlotAxis::tickLabel(double val) const
{
    char sbuf[32];
    if (d->m_labelFmt == 't') {
        while (val < 0.0) {
            val += 24.0;
        }
        while (val >= 24.0) {
            val -= 24.0;
        }
        const int h = int(val);
        const int m = int(60. * (val - h));
	sprintf(sbuf,"%02d:%02d",h,m);
	return std::string(sbuf);
    }
    switch(d->m_labelFmt){
    case 'g':sprintf(sbuf,"%g",val); break;
    case 'G':sprintf(sbuf,"%G",val); break;//precision(10) ; break;
    case 'e':sprintf(sbuf,"%e",val); break;
    case 'E':sprintf(sbuf,"%E",val); break;
    }
    return std::string(sbuf);//QStringLiteral("%1").arg(val, d->m_labelFieldWidth, d->m_labelFmt, d->m_labelPrec);
}

std::list<double> PlotAxis::majorTickMarks() const
{
    return d->m_MajorTickMarks;
}

std::list<double> PlotAxis::minorTickMarks() const
{
    return d->m_MinorTickMarks;
}

}//endof namespace
