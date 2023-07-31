#pragma once

#include <random>

#include "CoreMinimal.h"


struct FOpenSkillGaussian
{
	FOpenSkillGaussian(const double InMean, const double InVariance);
	double Mean;
	double Variance;
	double StandardDeviation;

	double CDF(double X) const;
	double PDF(double X) const;
	double PPF(double X) const;
	FOpenSkillGaussian Scale(double C) const;

	FOpenSkillGaussian operator*(const FOpenSkillGaussian& Other) const;
	FOpenSkillGaussian operator*(double Number) const;
	FOpenSkillGaussian operator/(const FOpenSkillGaussian& Other) const;
	FOpenSkillGaussian operator/(double Number) const;
	FOpenSkillGaussian operator+(const FOpenSkillGaussian& Other) const;
	FOpenSkillGaussian operator-(const FOpenSkillGaussian& Other) const;
};

/**
 * 
 */
class FOpenSkillStatistics
{
public:
	static double ERFC(const double X);
	static double IERFC(const double X);
	static double PhiMajor(const double X);
	static double PhiMajorInverse(const double X);
	static double PhiMinor(const double X);
	static double V(const double X, const double T);
	static double W(const double X, const double T);
	static double VT(const double X, const double T);
	static double WT(const double X, const double T);
};
