#pragma once
#include "OpenSkillModeling.h"
#include "OpenSkillOptions.h"
#include "OpenSkillTypes.h"
#include "Containers/Array.h"

TArray<TArray<FOpenSkillRating>> FOpenSkillModeling::PlackettLuce(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options)
{
	const double Kappa = Options.Kappa;
	const TArray<FOpenSkillTeamRating> TeamRatings = GetTeamRatings(Teams, Ranks);
	const double C = GetC(TeamRatings, Options);
	const TArray<double> SumQ = GetSumQ(TeamRatings, C);
	const TArray<double> A = GetA(TeamRatings);

	TArray<TArray<FOpenSkillRating>> Result;
	Result.Reserve(Teams.Num());

	for (int i = 0; i < TeamRatings.Num(); ++i)
	{
		const FOpenSkillTeamRating& TeamI = TeamRatings[i];
		const double TeamMuOverCe = FMath::Exp(TeamI.Mu / C);
		double OmegaSum = 0;
		double DeltaSum = 0;

		for (int q = 0; q < TeamRatings.Num(); ++q)
		{
			const FOpenSkillTeamRating& TeamQ = TeamRatings[q];
			if (TeamQ.Rank <= TeamI.Rank)
			{
				const double Quotient = TeamMuOverCe / SumQ[q];
				OmegaSum += (i == q ? 1 - Quotient : -Quotient) / A[q];
				DeltaSum += (Quotient * (1 - Quotient)) / A[q];
			}
		}
		const double IGamma = Options.Gamma(C, TeamRatings.Num(), TeamI.Mu, TeamI.SigmaSq, TeamI.Members, TeamI.Rank);
		const double IOmega = OmegaSum * (TeamI.SigmaSq / C);
		const double IDelta = IGamma * DeltaSum * (TeamI.SigmaSq / FMath::Square(C));

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
