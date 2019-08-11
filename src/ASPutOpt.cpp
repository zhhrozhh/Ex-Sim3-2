#include "ASPutOpt.h"
#include "Path.h"
#include "Util.h"
ASPutOpt::ASPutOpt(double T, double K) : Option(T)
{
	K_ = K;
}

ASPutOpt::ASPutOpt(Arguments& paras) : Option(paras)
{
	K_ = paras.g_VAL<double>("K");
}

double ASPutOpt::payoff(Arguments& paras)
{
	Path* path_ = paras.g_PTR<Path>("path");

	return UF::max(0.0, K_ - path_->arithmeticAvg());
}