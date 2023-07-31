#include "OpenSkillStatistics.h"
#include <random>

// Complementary error function
double FOpenSkillStatistics::ERFC(const double X)
{
	const double Z = FMath::Abs(X);
	const double T = 1 / (1 + Z / 2);
	const double R = T * FMath::Exp(-Z * Z - 1.26551223 + T * (1.00002368 +
		T * (0.37409196 + T * (0.09678418 + T * (-0.18628806 +
			T * (0.27886807 + T * (-1.13520398 + T * (1.48851587 +
				T * (-0.82215223 + T * 0.17087277)))))))));

	return X >= 0 ? R : 2 - R;
}

// Inverse complementary error function
double FOpenSkillStatistics::IERFC(const double X)
{
	if (X >= 2) { return -100; }
	if (X <= 0) { return 100; }

	const double XX = (X < 1) ? X : 2 - X;
	const double T = FMath::Sqrt(-2 * FMath::Loge(XX / 2));

	double R = -0.70711 * ((2.30753 + T * 0.27061) /
		(1 + T * (0.99229 + T * 0.04481)) - T);

	for (int j = 0; j < 2; j++)
	{
		const double Err = ERFC(R) - XX;
		R += Err / (1.12837916709551257 * FMath::Exp(-(R * R)) - R * Err);
	}

	return (X < 1) ? R : -R;
}

double FOpenSkillStatistics::PhiMajor(const double X)
{
	static FOpenSkillGaussian Normal(0, 1);
	return Normal.CDF(X);
}

double FOpenSkillStatistics::PhiMajorInverse(const double X)
{
	static FOpenSkillGaussian Normal(0, 1);
	return Normal.PPF(X);
}

double FOpenSkillStatistics::PhiMinor(const double X)
{
	static FOpenSkillGaussian Normal(0, 1);
	return Normal.PDF(X);
}

double FOpenSkillStatistics::V(const double X, const double T)
{
	const double XT = X - T;
	const double Denom = PhiMajor(XT);
	return Denom < DBL_EPSILON ? -XT : PhiMinor(XT) / Denom;
}

double FOpenSkillStatistics::W(const double X, const double T)
{
	const double XT = X - T;
	const double Denom = PhiMajor(XT);
	if (Denom < DBL_EPSILON)
	{
		return X < 0 ? 1 : 0;
	}
	return V(X, T) * (V(X, T) + XT);
}

double FOpenSkillStatistics::VT(const double X, const double T)
{
	const double XX = FMath::Abs(X);
	const double B = PhiMajor(T - XX) - PhiMajor(-T - XX);
	if (B < 1e-5)
	{
		return X < 0 ? -X - T : -X + T;
	}
	const double A = PhiMinor(-T - XX) - PhiMinor(T - XX);
	return (X < 0 ? -A : A) / B;
}

double FOpenSkillStatistics::WT(const double X, const double T)
{
	const double XX = FMath::Abs(X);
	const double B = PhiMajor(T - XX) - PhiMajor(-T - XX);
	return B < DBL_EPSILON
		       ? 1.0
		       : ((T - XX) * PhiMinor(T - XX) + (T + XX) * PhiMinor(-T - XX)) / B + VT(X, T) * VT(X, T);
}

FOpenSkillGaussian::FOpenSkillGaussian(const double InMean, const double InVariance)
{
	checkf(InVariance>0, TEXT("Variance must be > 0 (but was '%f')"), InVariance)

	Mean = InMean;
	Variance = InVariance;
	StandardDeviation = FMath::Sqrt(InVariance);
}

double FOpenSkillGaussian::CDF(const double X) const
{
	return 0.5 * FOpenSkillStatistics::ERFC(-(X - Mean) / (StandardDeviation * FMath::Sqrt(2)));
}

double FOpenSkillGaussian::PDF(double X) const
{
	double M = StandardDeviation * FMath::Sqrt(2 * PI);
	double E = FMath::Exp(-FMath::Pow(X - Mean, 2) / (2 * Variance));
	return E / M;
}

double FOpenSkillGaussian::PPF(const double X) const
{
	return Mean - StandardDeviation * FMath::Sqrt(2) * FOpenSkillStatistics::IERFC(2 * X);
}

FOpenSkillGaussian FOpenSkillGaussian::Scale(const double C) const
{
	return FOpenSkillGaussian(Mean * C, Variance * C * C);
}

FOpenSkillGaussian FOpenSkillGaussian::operator*(const FOpenSkillGaussian& Other) const
{
	const double Precision = 1 / Variance;
	const double OtherPrecision = 1 / Other.Variance;
	return FOpenSkillGaussian(Precision + OtherPrecision, Precision * Mean + OtherPrecision * Other.Mean);
}

FOpenSkillGaussian FOpenSkillGaussian::operator*(const double Number) const
{
	return Scale(Number);
}

FOpenSkillGaussian FOpenSkillGaussian::operator/(const FOpenSkillGaussian& Other) const
{
	const double Precision = 1 / Variance;
	const double OtherPrecision = 1 / Other.Variance;
	return FOpenSkillGaussian(Precision - OtherPrecision, Precision * Mean - OtherPrecision * Other.Mean);
}

FOpenSkillGaussian FOpenSkillGaussian::operator/(double Number) const
{
	return Scale(1 / Number);
}

FOpenSkillGaussian FOpenSkillGaussian::operator+(const FOpenSkillGaussian& Other) const
{
	return FOpenSkillGaussian(Mean + Other.Mean, Variance + Other.Variance);
}

FOpenSkillGaussian FOpenSkillGaussian::operator-(const FOpenSkillGaussian& Other) const
{
	return FOpenSkillGaussian(Mean - Other.Mean, Variance + Other.Variance);
}
