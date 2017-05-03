#ifndef _PULSE__
#define _PULSE__

#include "TF1.h"

double pulse_raw (double tau,
                  double x, double y, double a_0, double s, double t_0, double t)
{
    double result1, result2, result3;
    t -= t_0;

    // term 2
    result1 = tau * y * exp (-t / y);
    result1 /= pow (y, 2) - (x + tau) * y + tau * x;

    // term 3
    result2 = tau * x * exp (-t / x);
    result2 /= pow (x, 2) - (x - tau) * y - tau * x;

    //term 1
    result3 = pow (tau, 2) * exp (-t / tau);
    result3 /= pow (tau, 2) + (x - tau) * y - tau * x;
    return a_0 + s * (result1 + result2 + result3);
}

double pulse_x0 (double tau,
                 double y, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    return a_0 + s * (tau / (y - tau) * (exp (-t / y) - exp (-t / tau) ) );
}

double pulse_ytau (double tau,
                   double x, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    double result1, result2;
    result1 = exp (-t / x) - exp (-t / tau);
    result1 *= tau * x / (tau - x);
    result2 = t * exp (-t / tau);
    return a_0 + s * ( (result1 + result2) / (tau - x) );
}

double pulse_x0_ytau (double tau, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    return a_0 + s * (t / tau * exp (-t / tau) );
}

double pulse_xy (double tau, double x, double a_0, double s, double t_0, double t)
{
    t -= t_0;
    return a_0 + s * (pow (tau, 2) * exp (-t / tau) / (pow ( (x - tau), 2) ) );
}

double pulse (double tau,
              double x, double y, double a_0, double s, double t_0, double t)
{
    if (x > y)
    {
        double pivot = x;
        x = y;
        y = pivot;
    }

    if ( (x == 0) && (y == tau) ) return pulse_x0_ytau (tau, a_0, s, t_0, t);
    //added by Georg
    //else if (x == y) return pulse_xy (tau, x, a_0, s, t_0, t );
    else if (x == 0) return pulse_x0 (tau, y, a_0, s, t_0, t);
    else if (y == tau) return pulse_ytau (tau, x, a_0, s, t_0, t);
    else return pulse_raw (tau, x, y, a_0, s, t_0, t);
}

double fpulse (double* x, double* par)
{
    double xx = par[0];
    double y = par[1];
    double tau = par[2];
    double a_0 = par[3];
    double s = par[4];
    double t_0 = par[5];

    if (x[0] < t_0) return a_0;

    return pulse (tau, xx, y, a_0, s, t_0, x[0]);
}

double fpulsedeconv (double* x, double* par)
{
    double xm = par[6] * (x[0] - 25);
    double xp = par[6] * (x[0] + 25);
    double xz = par[6] * x[0];
    return 1.2131 * fpulse (&xp, par) - 1.4715 * fpulse (&xz, par) + 0.4463 * fpulse (&xm, par);
}

//void fit (TH1* pHist)
//{

//TF1* a = new TF1 ("test", fpulse, 0, 200, 6);
//a->SetParNames ("PA RC time constant x      ", "shaper RC time constant y  ", "shaper CR time constant tau", "baseline                   ", "scale                      ", "turn on time               " );
//a->SetParLimits (0, 1, 30); //x
//a->SetParLimits (1, 1, 30); //y
//a->FixParameter (2, 50);//tau
//a->SetParLimits (3, -500, 500);//baseline
//a->SetParLimits (4, 0, 10000 );//scale
//a->SetParLimits (5, 0, 50); // turn-on time
//a->SetParameters (15, 30, 50, 180, 560, 23);
//pHist->Fit (a, "RM+");
//pHist->Draw();
//std::cout << a->GetChisquare() / a->GetNDF() << std::endl;
//}

//double ftest (double tau, double x, double a_0, double s, double t_0, double t)
//{
//double result1, result2, result3;
//tau -= t_0;

//// term 2
//result1 = tau * x * exp (-t / x);
//result1 /= pow (x, 2) - 2 * (x * tau)  + pow (tau, 2);

//// term 2
//result2 = tau * x * exp (-t / tau);
//result2 /= pow (x, 2) - 2 * (x * tau) * pow (tau, 2);

////term 1
//result3 = t * exp (-t / tau);
//result3 /= x - tau;
//return a_0 + s * (result1 - result2 - result3);
//}

//double f_test (double* x, double* par)
//{
//double xx = par[0];
//double tau = par[1];
//double a_0 = par[2];
//double s = par[3];
//double t_0 = par[4];

//if (x[0] < t_0) return a_0;

//return ftest (tau, xx, a_0, s, t_0, x[0]);
//}

//void pulse()
//{
//TF1* a = new TF1 ("a", f_test, 0, 200, 5) ;
////f_peak->SetParameters (9, 35, 50, 0, 5600, 24);
//a->SetParameters (9, 50, 0, 5600, 24);
//a->Draw();
//}

#endif
