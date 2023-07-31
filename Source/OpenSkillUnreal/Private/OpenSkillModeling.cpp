#include "OpenSkillModeling.h"
#include "OpenSkillOptions.h"

double FOpenSkillModeling::GetScore(double Q, double I)
{
	if (Q < I)
	{
		return 0.0;
	}
	if (Q > I)
	{
		return 1.0;
	}
	return 0.5;
}

TArray<int> FOpenSkillModeling::GetRankings(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks)
{
	TArray<int> TeamScores;
	TeamScores.Reserve(Teams.Num());
	TArray<int> OutRank;
	OutRank.SetNumZeroed(Teams.Num());

	for (int i = 0; i < Teams.Num(); ++i)
	{
		if (Ranks.Num() > i)
		{
			TeamScores.Add(Ranks[i]);
		}
		else
		{
			TeamScores.Add(i);
		}
	}

	int s = 0;
	for (int j = 0; j < TeamScores.Num(); ++j)
	{
		if (j > 0 && TeamScores[j] - 1 < TeamScores[j])
		{
			s = j;
		}
		OutRank[j] = s;
	}
	return OutRank;
}

FOpenSkillTeamRating FOpenSkillModeling::GetTeamRating(const TArray<FOpenSkillRating>& Team, const int Rank)
{
	double Mu = 0;
	double Sigma = 0;

	for (const FOpenSkillRating& Member : Team)
	{
		Mu += Member.Mu;
		Sigma += FMath::Square(Member.Sigma);
	}
	return FOpenSkillTeamRating(Mu, Sigma, Team, Rank);
}

TArray<FOpenSkillTeamRating> FOpenSkillModeling::GetTeamRatings(const TArray<TArray<FOpenSkillRating>>& Game, const TArray<int>& Ranks)
{
	TArray<int> Rankings = GetRankings(Game, Ranks);
	TArray<FOpenSkillTeamRating> Result;

	for (int i = 0; i < Game.Num(); ++i)
	{
		const TArray<FOpenSkillRating>& Team = Game[i];
		Result.Emplace(GetTeamRating(Team, Rankings[i]));
	}

	return Result;
}

double FOpenSkillModeling::GetC(const TArray<FOpenSkillTeamRating>& TeamRatings, const FOpenSkillOptions& Options)
{
	const double BetaSquared = FMath::Square(Options.Beta);
	double TeamSigmaSq = 0;
	for (const FOpenSkillTeamRating& TeamRating : TeamRatings)
	{
		TeamSigmaSq += TeamRating.SigmaSq + BetaSquared;
	}
	return FMath::Sqrt(TeamSigmaSq);
}

TArray<double> FOpenSkillModeling::GetSumQ(const TArray<FOpenSkillTeamRating>& TeamRatings, const double C)
{
	TArray<double> Result;

	for (int q = 0; q < TeamRatings.Num(); ++q)
	{
		double Sum = 0;
		const FOpenSkillTeamRating& TeamQ = TeamRatings[q];
		for (int i = 0; i < TeamRatings.Num(); ++i)
		{
			const FOpenSkillTeamRating& TeamI = TeamRatings[i];
			if (TeamI.Rank >= TeamQ.Rank)
			{
				Sum += FMath::Exp(TeamI.Mu / C);
			}
		}
		Result.Add(Sum);
	}
	return Result;
}

TArray<double> FOpenSkillModeling::GetA(const TArray<FOpenSkillTeamRating>& TeamRatings)
{
	TArray<double> Result;

	for (int q = 0; q < TeamRatings.Num(); ++q)
	{
		int MatchingRanks = 0;
		const FOpenSkillTeamRating& TeamQ = TeamRatings[q];
		for (int i = 0; i < TeamRatings.Num(); ++i)
		{
			const FOpenSkillTeamRating& TeamI = TeamRatings[i];
			if (TeamI.Rank == TeamQ.Rank)
			{
				MatchingRanks++;
			}
		}
		Result.Add(MatchingRanks);
	}
	return Result;
}
