#include <cmath>
#include <TGraph.h>
#include <iostream>
#include <cstdio>
#include <TFitResult.h>
#include <TFitResultPtr.h>

TGraph* myPulse = NULL;

/*
   antani.SetPoint(0,0,0);
   for(int i=1; i<200; ++i) antani.SetPoint(i, i, pulse(50, 5, 50, i));
   antani.Draw("alp");
 */

TGraph antani;

/*
   double pulse(double tau,
   double x, double y, double t) {
   double result1, result2, result3;
   double y2 = y*y;
   double x2 = x*x;
   double taux = tau*x;
   result1 = y2 - y*(x+tau) + taux;
   result1 = tau*y*exp(-t/y)/result1;
   result2 = (x-tau)*y-x2+taux;
   result2 = taux*exp(-t/x)/result2;
   result3 = (x-tau)*y-taux+tau*tau;
   result3 = tau*tau*exp(-t/tau)/result3;
   return result1-result2+result3;
   }
 */

double pulse_raw (double tau,
                  double x, double y, double t)
{
    double result1, result2, result3;

    // term 2
    result1 = tau * y * exp (-t / y);
    result1 /= pow (y, 2) - (x + tau) * y + tau * x;

    // term 3
    result2 = tau * x * exp (-t / x);
    result2 /= pow (x, 2) - (x - tau) * y - tau * x;

    //term 1
    result3 = pow (tau, 2) * exp (-t / tau);
    result3 /= pow (tau, 2) + (x - tau) * y - tau * x;
    return result1 + result2 + result3;
}

double pulse_x0 (double tau,
                 double y, double t)
{
    return tau / (y - tau) * (exp (-t / y) - exp (-t / tau) );
}

double pulse_ytau (double tau,
                   double x, double t)
{
    double result1, result2;
    result1 = exp (-t / x) - exp (-t / tau);
    result1 *= tau * x / (tau - x);
    result2 = t * exp (-t / tau);
    return (result1 + result2) / (tau - x);
}

double pulse_x0_ytau (double tau, double t)
{
    return t / tau * exp (-t / tau);
}

double pulse (double tau,
              double x, double y, double t)
{
    if (x > y)
    {
        double pivot = x;
        x = y;
        y = pivot;
    }

    if ( (x == 0) && (y == tau) ) return pulse_x0_ytau (tau, t);
    else if (x == 0) return pulse_x0 (tau, y, t);
    else if (y == tau) return pulse_ytau (tau, x, t);
    else return pulse_raw (tau, x, y, t);
}

void printValues (double tau, double x, double y)
{
    antani.SetPoint (0, 0, 0);
    double v;

    for (int t = 1; t < 200; ++t)
    {
        v = pulse (tau, x, y, double (t) );
        printf ("t=%f\tv=%f\n", double (t), v);
        antani.SetPoint (t, t, v);
    }

    antani.Draw ("alp");
}

double find_maximum (double tau, double x, double y)
{
    double current_t = tau;
    double current_y = pulse (tau, x, y, current_t);
    double max_t = current_t;
    double max_y = current_y;
    double last_y = 0;

    while (current_y > last_y)
    {
        current_t += maxtime / npoints;
        last_y = current_y;
        current_y = pulse (tau, x, y, current_t);

        if (current_y > max_y)
        {
            max_y = current_y;
            max_t = current_t;
        }

        //printf ("current_t=%f, current_y=%f, last_y=%f\n", current_t, current_y, last_y);
    }

    last_y = current_y - 1e-6;

    while (current_y > last_y)
    {
        current_t -= maxtime / npoints;
        last_y = current_y;
        current_y = pulse (tau, x, y, current_t);

        if (current_y > max_y)
        {
            max_y = current_y;
            max_t = current_t;
        }

        //printf ("current_t=%f, current_y=%f, last_y=%f\n", current_t, current_y, last_y);
    }

    return max_t;
}

double adjust_maximum (double tau, double x)
{
    double y = tau - x;
    double last_max, move;

    for (int i = 0; i < 20; ++i)
    {
        last_max = find_maximum (tau, x, y);
        move = last_max - tau;
        y -= move;
        //printf("y=%.6f\n",y);
    }

    return y;
}

void make_maxima (double tau)
{
    double y;

    if (myPulse) delete myPulse;

    myPulse = new TGraph();
    int i = 0;
    myPulse->SetPoint (i++, 0, 50);

    for (double x = 1.01; x < 50; x += 0.1)
    {
        y = adjust_maximum (tau, x) ;
        myPulse->SetPoint (i++, x, y);
    }
}


void make_pulse (double tau, double x, double y)
{
    if (myPulse) delete myPulse;

    myPulse = new TGraph (npoints);
    myPulse->SetPoint (0, 0, 0);
    double x_v;

    for (int i = 1; i < npoints; ++i)
    {
        x_v = i * maxtime / npoints;
        myPulse->SetPoint (i, x_v, pulse (tau, x, y, x_v) );
    }
}

double get_compensation (double x)
{
    return 49.9581
           - 1.7941 * x
           - 0.110089 * pow (x, 2)
           + 0.0113809 * pow (x, 3)
           - 0.000388111 * pow (x, 4)
           + 5.9761e-06 * pow (x, 5)
           - 3.51805e-08 * pow (x, 6);
}

void make_peakshift (double tau, double C_fd, double preampShift)
{
    const int nPoints_shift = 50;

    if (myPulse) delete myPulse;

    myPulse = new TGraph (nPoints_shift);
    double x = C_fd * preampShift;
    double y = adjust_maximum (tau, x);
    double C_added;

    for (int i = 0; i < nPoints_shift; ++i)
    {
        C_added = double (i) * 2;
        x = (C_fd + C_added) * preampShift;
        myPulse->SetPoint (i, C_added, find_maximum (tau, x, y) - tau);
    }

    myPulse->Draw ("alp");
    TFitResultPtr myResultPtr = myPulse->Fit ("pol2", "NSQ"); // NSQ
    TFitResult* myResult = myResultPtr.Get();
    std::cout << "Slope = " << myResult->Parameter (1) << std::endl;

}

void peak()
{
    double tau = 5.;
    double x = 40;
    double y = 50;
    //make_pulse (tau, x, y);
    printValues (tau, x, y);
    //myPulse->Draw ("alp");
}

/*
// test me:
.L peak.cpp+
double tau=50;
double x=4;
double y;
y=adjust_maximum(tau,x); make_pulse(tau,x,y); myPulse->Draw("alp"); printf("max at %f\n", find_maximum(tau,x,y));

//find_maximum(tau,x,y)
adjust_maximum_2(tau,x)
make_maxima(tau);
myPulse->Draw("alp");

y = 49.9581 -1.7941 * x -0.110089 * pow(x,2) +0.0113809 * pow(x,3) -0.000388111* pow(x,4) +5.9761e-06 * pow(x,5) -3.51805e-08 * pow(x,6);

****************************************
Minimizer is Linear
Chi2                      =      2.30948
NDf                       =          484
p0                        =      49.9581   +/-   0.0308837
p1                        =      -1.7941   +/-   0.0153878
p2                        =    -0.110089   +/-   0.00250554
p3                        =    0.0113809   +/-   0.000180896
p4                        = -0.000388111   +/-   6.41646e-06
p5                        =   5.9761e-06   +/-   1.0953e-07
p6                        = -3.51805e-08   +/-   7.18719e-10



 */
