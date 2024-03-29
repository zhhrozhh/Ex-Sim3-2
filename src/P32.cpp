//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32.h
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#define _USE_MATH_DEFINES
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#include "P32.h"
#include "Util.h"
#include "Exceptions.h"
#include "Path.h"

#include <cmath>
#include <cassert>
#include <vector>

#include <iostream>
#include <fstream>
const double pidd2 = 0.63661977236758134307553505349005744813783858296182579499066937623;
const double tol_eps = 0.001570796326794896619231321691639751442098584699687552910487472296;
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::P32
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
P32::P32
(
    double r,
    double rho,
    double kappa,
    double theta,
    double epsilon,
    double dt
) : Process(dt)
{
    r_ = r;
    rho_ = rho;
    kappa_ = kappa;
    theta_ = theta;    
    epsilon_ = epsilon;
    para_validate();
    set_loaded(false);
    post_update();
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::P32
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
P32::P32(Arguments& paras) : Process(paras)
{
    r_ = paras.g_VAL<double>("r");
    rho_ = paras.g_VAL<double>("rho");
    kappa_ = paras.g_VAL<double>("kappa");
    theta_ = paras.g_VAL<double>("theta");
    epsilon_ = paras.g_VAL<double>("epsilon");
    S0_ = -1.0;
    V0_ = -1.0;
    VT_ = -1.0;
    para_validate();
    try
    {
        S0_ = paras.g_VAL<double>("S0");
        V0_ = paras.g_VAL<double>("V0");
        set_loaded(true);
    }
    catch(...){set_loaded(false);}
    post_update();
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::para_validate
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void P32::para_validate()
{
    Process::para_validate();
    assert(r_ >= 0.0);
    assert(rho_ >= -1.0);
    assert(rho_ <= 1.0);
    assert(kappa_ >= 0.0);
    assert(theta_ >= 0.0);
    assert(epsilon_ > 0.0);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::post_update
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void P32::post_update()
{

    double T = get_dt();
    eps2_ = epsilon_ * epsilon_;
    p_ = - (2.0 * kappa_ * theta_) / eps2_;
    v_ = 2.0 * kappa_ * theta_ * (kappa_ + eps2_) / (eps2_ * kappa_ * theta_) - 1.0;
    Delta_ = 0.25 * T * eps2_; 
    delta_ = 4.0 * (eps2_ + kappa_) / eps2_;
    ektT_ = std::exp(kappa_ * theta_ * T);
    zp_ = eps2_ * (ektT_-1.0) / (4.0 * kappa_ * theta_);

    if(check_loaded())
    {
        X0_ = 1.0 / V0_;
        lambda_ = X0_ / zp_;
    }

}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::para_load
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void P32::para_load(Arguments& paras)
{
    S0_ = paras.g_VAL<double>("S0");
    V0_ = paras.g_VAL<double>("V0");
    set_loaded(true);
    post_update();
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::simulate
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double P32::simulate()
{
    if((S0_ < 0.0) || (V0_ < 0.0)) throw PRONotLoaded_Exception();
    double T = get_dt();
    double Z;
    size_t err_count = 0;
    while(err_count < 100)
    {
        try
        {
            Z = UF::ncChi2Rnd(delta_, lambda_);
            break;
        }
        catch(NC_Exception& e)
        {
            if(err_count >= 98)
                throw NCDead_Exception(delta_, lambda_);
        }
        err_count += 1;
    }
    
    double XT = Z * zp_/ektT_;
    VT_ = 1.0 / XT;

    double x = p_ * std::sqrt(XT * X0_)/std::sinh(p_ * Delta_);
    double v = v_;
    double eps2 = eps2_;
    double duii = std::real(UF::I(std::abs(v), x));
    double dui = 1.0 / duii;
    std::function<std::complex<double>(double)> Phi = 
        [&, v, x, eps2, dui](double a) -> std::complex<double>
    {
        return UF::I
        (
            std::sqrt(std::complex<double>(v * v, - 8.0 * a / eps2)),
            x
        ) * dui;
    };
    double mu = 
    (
        std::real(std::complex<double>(0.0, -1.0) * 
        UF::numericalDiff(Phi, 0.0, 0.01))
    );
    double sigma2 = std::real(-UF::numericalDiff2(Phi, 0.0, 0.01)) - mu * mu;


    double sigma = std::sqrt(sigma2);
    double ueps = mu + 12.0 * sigma;
    double h = M_PI / ueps;

    std::function<double(double)> F = [&, h, Phi](double x)->double
    {
        double res = h * x / M_PI;
        double i = 1.0;
        double res2 = 0.0;
        std::complex<double> phidi;
        do
        {
            phidi = Phi(h * i) / i;
            res2 += std::sin(h * i * x) * std::real(phidi);
            i += 1.0;
        }
        while(std::abs(phidi) / i > tol_eps);
        return res + pidd2 * res2;
    };

    double L = UF::rvs(F, UF::uniRnd(0.0, 1.0), 1.0, 0.0, ueps, mu);
    double K = 1.0 / epsilon_ * (std::log(X0_ / XT) + (kappa_ + 0.5 * eps2_) * L - T * kappa_ * theta_);

    double m = std::log(S0_) + r_ * T - 0.5 * L + rho_ * K;
    double s = (1.0 - rho_ * rho_) * L;

    double ZZ = UF::normalRnd(m, s);
    return std::exp(ZZ);
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::simulate
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double P32::simulate(Arguments& paras)
{
    para_load(paras);
    double* res = new double(simulate());
    paras.g_SET<double>("ST", res);
    return *res;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  P32::simulatePath
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
Path * P32::simulatePath(Arguments& paras)
{
    std::vector<double> path;
    double S0_backup = S0_;
    double V0_backup = V0_;
    double St = S0_;
    double Vt = V0_;
    double T = paras.g_VAL<double>("T");
    path.push_back(St);
    double dt = get_dt();
    for(double tt = 0.0; tt <= T; tt += dt)
    {
        paras.g_SET<double>("S0", new double(St));
        paras.g_SET<double>("V0", new double(Vt));
        para_load(paras);

        St = simulate();

        path.push_back(St);
        Vt = VT_;
        if((St != St) || (Vt != Vt)) throw PTHAbnormal_Exception();
    }
    paras.g_SET<double>("S0", new double(S0_backup));
    paras.g_SET<double>("V0", new double(V0_backup));
    paras.g_SET<double>("ST", new double(St));
    S0_ = S0_backup;
    V0_ = V0_backup;
    Path* path_ = new Path(0.0, dt, T, path);
    paras.g_SET<Path>("path", path_);

    return path_;
}




//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  End
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
