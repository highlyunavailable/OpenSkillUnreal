#pragma once

struct FOpenSkillOptions;
struct FOpenSkillRating;

typedef TFunction<double(const double, const double, const double, const double, const TArray<FOpenSkillRating>&, const double)> FOpenSkillGamma;
typedef TFunction<TArray<TArray<FOpenSkillRating>>(const TArray<TArray<FOpenSkillRating>>&, const TArray<int>&, const FOpenSkillOptions&)> FOpenSkillModel;

struct FOpenSkillRating
{
	FOpenSkillRating(const double InMu, const double InSigma)
	{
		Mu = InMu;
		Sigma = InSigma;
	}

	double Mu = 0;
	double Sigma = 0;
};

struct FOpenSkillTeamRating
{
	FOpenSkillTeamRating(const double InMu, const double InSigmaSq, const TArray<FOpenSkillRating>& InTeam, const int InRank)
	{
		Mu = InMu;
		SigmaSq = InSigmaSq;
		Members = InTeam;
		Rank = InRank;
	}

	double Mu;
	double SigmaSq;
	TArray<FOpenSkillRating> Members;
	double Rank;
};
