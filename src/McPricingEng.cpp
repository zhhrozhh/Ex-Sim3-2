//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  McPricingEng.cpp
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#include "Process.h"
#include "Option.h"
#include "McPricingEng.h"
#include "Exceptions.h"
#include "CSVLogger.h"

#include <cmath>
#include <functional>
#include <vector>
#include <ctime>

#include <stdio.h>

#include <iostream>
#include <fstream>
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  McPricingEng::McPricingEng
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
McPricingEng::McPricingEng(Option*opt, Process*pro) : PricingEng(opt){pro_ = pro;}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  McPricingEng::McPricingEng
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
McPricingEng::McPricingEng(Arguments& paras) : PricingEng(paras)
{
    pro_ = paras.g_PTR<Process>("process");
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  McPricingEng::price
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double McPricingEng::price(){throw BadAccess_Exception();}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  McPricingEng::price
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
double McPricingEng::price(Arguments& paras)
{
    clock_t t0 = std::clock();
    size_t bsize = paras.g_VAL<size_t>("nos");
    double T = paras.g_VAL<double>("T");
    double r = paras.g_VAL<double>("r");
    double res = 0.0;
    bool verbose = false;
    double var2 = 0.0;
    double poff = 0.0;
    bool dbg = false;
    bool simp = false;
    double* row = new double[4];

    if(paras.has("simp"))
        simp = paras.g_VAL<bool>("simp");

    std::ofstream* dbgofs = nullptr;
    if(paras.has("verbose"))
        verbose = paras.g_VAL<bool>("verbose");

    if(verbose)
    {
        std::cout << "simulating..." << std::endl;
        printf("------");
        std::cout << std::flush;
    }
    if(paras.has("dbg"))
    {
        dbg = true;
        dbgofs = paras.g_PTR<std::ofstream>("dbg");
        (*dbgofs) << "[";
    }
    for(size_t i = 0; i < bsize; i++)
    {
        if(verbose && (!(i%50)))
        {
            printf("\b\b\b\b\b\b");
            printf("%05.2f%%", (double)i / (double)bsize * 100.0);
            std::cout << std::flush;
        }
        if(simp) pro_ -> simulate(paras);
        else pro_->simulatePath(paras);
        poff = opt_ -> payoff(paras);
        if(i > 1)
            var2 = (
                ((double)i - 1.0) / ((double)i) * var2 
                +
                (poff - res / (double)(i)) * (poff - res / (double)(i)) / (double)(i + 1)
            );
        res += poff;
        if(dbg)
            (*dbgofs) << poff << ",";
    }
    if(dbg)
        (*dbgofs) << "]" << std::endl << std::endl;
    res = res * std::exp(-r * T) / (double) (bsize);
    clock_t dt = std::clock() - t0;
    if(verbose)
    {
        printf("\b\b\b\b\b\b");
        *row = (double)bsize;
        *(row + 1) = (double)dt / CLOCKS_PER_SEC;
        *(row + 2) = res;
        *(row + 3) = std::sqrt(var2) * std::exp(-r * T) / std::sqrt(bsize);
        if(paras.has("obv"))
            *(row + 4) = paras.g_VAL<double>(paras.g_VAL<std::string>("obv"));
        std::cout << "100%--" << std::endl;
        std::cout << "done." << std::endl;
        std::cout << "result: " << res << std::endl;
        std::cout << "std err: " << *(row + 3) << std::endl;
        std::cout << "time usage: " << *(row + 1) << std::endl;
        std::cout << "avg sims per sec: " << (double)bsize * CLOCKS_PER_SEC / (double)dt;
        std::cout << std::endl;
        std::cout << std::endl;
        try
        {
            paras.g_PTR<CSVLogger>("csv") -> add(row);
        }
        catch(...){}
    }

    return res;
}
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//  End
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
