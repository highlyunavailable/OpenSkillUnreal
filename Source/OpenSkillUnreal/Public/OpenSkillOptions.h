#pragma once
#include "OpenSkillTypes.h"
#include "OpenSkillModeling.h"

struct FOpenSkillOptions
{
	double Z = 3.0;
	double Mu = 25.0;
	double Sigma = Mu / Z;
	double Kappa = 0.0001;
	FOpenSkillGamma Gamma = &FOpenSkillModeling::DefaultGamma;
	double Beta = Sigma / 2;
	double Tau = Mu / 300;
	FOpenSkillModel Model = &FOpenSkillModeling::PlackettLuce;
	bool PreventSigmaIncrease = false;
};
