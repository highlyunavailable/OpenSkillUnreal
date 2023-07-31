#pragma once
#include "OpenSkillModeling.h"
#include "OpenSkillOptions.h"
#include "OpenSkillTypes.h"
#include "Containers/Array.h"

TArray<TArray<FOpenSkillRating>> FOpenSkillModeling::BradleyTerryFull(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options)
{
	const double Kappa = Options.Kappa;
	const double TwoBetaSq = 2 * FMath::Square(Options.Beta);
	const TArray<FOpenSkillTeamRating> TeamRatings = GetTeamRatings(Teams, Ranks);
	const double C = GetC(TeamRatings, Options);
	const TArray<double> SumQ = GetSumQ(TeamRatings, C);
	const TArray<double> A = GetA(TeamRatings);

	TArray<TArray<FOpenSkillRating>> Result;
	Result.Reserve(Teams.Num());

	for (int i = 0; i < TeamRatings.Num(); ++i)
	{
		const FOpenSkillTeamRating& TeamI = TeamRatings[i];
		double IOmega = 0;
		double IDelta = 0;

		for (int q = 0; q < TeamRatings.Num(); ++q)
		{
			if (q == i)
			{
				continue;
			}
			const FOpenSkillTeamRating& TeamQ = TeamRatings[q];

			const double Ciq = FMath::Sqrt(TeamI.SigmaSq + TeamQ.SigmaSq + TwoBetaSq);
			const double Piq = 1 / (1 + FMath::Exp((TeamQ.Mu - TeamI.Mu) / Ciq));
			const double QEta = TeamI.SigmaSq / Ciq;
			const double IGamma = Options.Gamma(C, TeamRatings.Num(), TeamI.Mu, TeamI.SigmaSq, TeamI.Members, TeamI.Rank);

			IOmega += QEta * (GetScore(TeamQ.Rank, TeamI.Rank) - Piq);
			IDelta += ((IGamma * QEta) / Ciq) * Piq * (1 - Piq);
		}

		TArray<FOpenSkillRating> Rated;
		Rated.Reserve(TeamI.Members.Num());
		for (const FOpenSkillRating& Member : TeamI.Members)
		{
			const double SigmaSq = FMath::Square(Member.Sigma);
			Rated.Emplace(Member.Mu + (SigmaSq / TeamI.SigmaSq) * IOmega,
			              Member.Sigma * FMath::Sqrt(FMath::Max(1 - (SigmaSq / TeamI.SigmaSq) * IDelta, Kappa)));
		}
		Result.Emplace(MoveTemp(Rated));
	}
	return Result;
}