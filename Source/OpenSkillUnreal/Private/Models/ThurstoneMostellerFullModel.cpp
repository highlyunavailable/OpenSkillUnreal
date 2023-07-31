#pragma once
#include "OpenSkillModeling.h"
#include "OpenSkillOptions.h"
#include "OpenSkillTypes.h"
#include "OpenSkillStatistics.h"
#include "Containers/Array.h"

TArray<TArray<FOpenSkillRating>> FOpenSkillModeling::ThurstoneMostellerFull(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options)
{
	const double Kappa = Options.Kappa;
	const double TwoBetaSq = 2 * FMath::Square(Options.Beta);
	const TArray<FOpenSkillTeamRating> TeamRatings = GetTeamRatings(Teams, Ranks);

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
			const double DeltaMu = (TeamI.Mu - TeamQ.Mu) / Ciq;
			const double SigSqToCiq = TeamI.SigmaSq / Ciq;
			const double IGamma = Options.Gamma(Ciq, TeamRatings.Num(), TeamI.Mu, TeamI.SigmaSq, TeamI.Members, TeamI.Rank);

			if (TeamQ.Rank == TeamI.Rank)
			{
				IOmega += SigSqToCiq * FOpenSkillStatistics::VT(DeltaMu, Kappa / Ciq);
				IDelta += ((IGamma * SigSqToCiq) / Ciq) * FOpenSkillStatistics::WT(DeltaMu, Kappa / Ciq);
			}
			else
			{
				const double Sign = TeamQ.Rank > TeamI.Rank ? 1 : -1;
				IOmega += Sign * SigSqToCiq * FOpenSkillStatistics::V(Sign * DeltaMu, Kappa / Ciq);
				IDelta += ((IGamma * SigSqToCiq) / Ciq) * FOpenSkillStatistics::W(Sign * DeltaMu, Kappa / Ciq);
			}
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
