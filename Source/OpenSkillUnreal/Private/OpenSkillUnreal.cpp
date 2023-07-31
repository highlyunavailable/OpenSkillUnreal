// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenSkillUnreal.h"
#include "OpenSkillStatistics.h"

#define LOCTEXT_NAMESPACE "FOpenSkillUnrealModule"

TArray<TArray<FOpenSkillRating>> FOpenSkillUnrealModule::RateByOrdinal(const TArray<TArray<FOpenSkillRating>>& Teams) const
{
	TArray<TArray<FOpenSkillRating>> ProcessedTeams = Teams;
	TArray<int> Ranks;
	Ranks.Reserve(Teams.Num());
	for (int i = 1; i <= Teams.Num(); ++i)
	{
		Ranks.Add(i);
	}
	return RateInternal(MoveTemp(ProcessedTeams), MoveTemp(Ranks), {});
}

TArray<TArray<FOpenSkillRating>> FOpenSkillUnrealModule::RateByRank(const TArray<TTuple<TArray<FOpenSkillRating>, int>>& Teams) const
{
	TArray<TArray<FOpenSkillRating>> ProcessedTeams;
	TArray<int> Ranks;
	ProcessedTeams.Reserve(Teams.Num());
	Ranks.Reserve(Teams.Num());
	for (const TTuple<TArray<FOpenSkillRating>, int>& Team : Teams)
	{
		ProcessedTeams.Emplace(Team.Key);
		Ranks.Emplace(Team.Value);
	}
	return RateInternal(MoveTemp(ProcessedTeams), MoveTemp(Ranks), {});
}

TArray<TArray<FOpenSkillRating>> FOpenSkillUnrealModule::RateByScore(const TArray<TTuple<TArray<FOpenSkillRating>, int>>& Teams) const
{
	TArray<TArray<FOpenSkillRating>> ProcessedTeams;
	TArray<int> InverseScores;
	ProcessedTeams.Reserve(Teams.Num());
	InverseScores.Reserve(Teams.Num());
	for (const TTuple<TArray<FOpenSkillRating>, int>& Team : Teams)
	{
		ProcessedTeams.Emplace(Team.Key);
		InverseScores.Emplace(-Team.Value);
	}
	return RateInternal(MoveTemp(ProcessedTeams), MoveTemp(InverseScores), {});
}

TArray<double> FOpenSkillUnrealModule::PredictWin(const TArray<TArray<FOpenSkillRating>>& Teams) const
{
	const double BetaSquared = FMath::Square(Options.Beta);
	const double N = Teams.Num();
	const double Denom = (N * (N - 1)) / 2;
	const TArray<FOpenSkillTeamRating> TeamRatings = FOpenSkillModeling::GetTeamRatings(Teams, {});

	TArray<double> Result;
	Result.Reserve(Teams.Num());

	for (int i = 0; i < TeamRatings.Num(); ++i)
	{
		const FOpenSkillTeamRating& TeamI = TeamRatings[i];

		double Prob = 0;
		for (int q = 0; q < TeamRatings.Num(); ++q)
		{
			if (i == q)
			{
				continue;
			}

			const FOpenSkillTeamRating& TeamQ = TeamRatings[q];
			Prob += FOpenSkillStatistics::PhiMajor((TeamI.Mu - TeamQ.Mu) / FMath::Sqrt(N * BetaSquared + FMath::Square(TeamI.SigmaSq) + FMath::Square(TeamQ.SigmaSq)));
		}
		Result.Emplace(Prob / Denom);
	}
	return Result;
}


double FOpenSkillUnrealModule::PredictDraw(const TArray<TArray<FOpenSkillRating>>& Teams) const
{
	const double Beta = Options.Beta;
	const double BetaSquared = FMath::Square(Options.Beta);
	const double N = Teams.Num();

	if (N == 0)
	{
		return 0;
	}
	if (N == 1)
	{
		return 1;
	}

	const double Denom = (N * (N - 1)) / (N > 2 ? 1 : 2);
	const TArray<FOpenSkillTeamRating> TeamRatings = FOpenSkillModeling::GetTeamRatings(Teams, {});

	double PlayerCount = 0;
	for (const TArray<FOpenSkillRating>& Players : Teams)
	{
		PlayerCount += Players.Num();
	}

	const double DrawMargin = FMath::Sqrt(PlayerCount) * Beta * FOpenSkillStatistics::PhiMajorInverse((1 + 1 / N) / 2);

	double Result = 0;

	for (int i = 0; i < TeamRatings.Num(); ++i)
	{
		const FOpenSkillTeamRating& TeamI = TeamRatings[i];
		for (int q = 0; q < TeamRatings.Num(); ++q)
		{
			if (i == q)
			{
				continue;
			}
			const FOpenSkillTeamRating& TeamQ = TeamRatings[q];
			const double SigmaBar = FMath::Sqrt(N * BetaSquared + FMath::Square(TeamI.SigmaSq) + FMath::Square(TeamQ.SigmaSq));
			Result += FOpenSkillStatistics::PhiMajor((DrawMargin - TeamI.Mu + TeamQ.Mu) / SigmaBar) - FOpenSkillStatistics::PhiMajor((TeamI.Mu - TeamQ.Mu - DrawMargin) / SigmaBar);
		}
	}
	return FMath::Abs(Result) / Denom;
}

TArray<TTuple<int, double>> FOpenSkillUnrealModule::PredictRank(const TArray<TArray<FOpenSkillRating>>& Teams) const
{
	if (Teams.Num() == 0)
	{
		return {};
	}
	if (Teams.Num() == 1)
	{
		return {TTuple<int, double>(1, 1.0)};
	}

	const double Beta = Options.Beta;
	const double BetaSquared = FMath::Square(Options.Beta);

	double PlayerCount = 0;
	for (const TArray<FOpenSkillRating>& Players : Teams)
	{
		PlayerCount += Players.Num();
	}

	const double N = Teams.Num();
	const double Denom = (N * (N - 1)) / 2;
	const double DrawMargin = FMath::Sqrt(PlayerCount) * Beta * FOpenSkillStatistics::PhiMajorInverse((1 + 1 / N) / 2);

	TArray<double> PairwiseProbabilities;
	PairwiseProbabilities.Reserve(N * N - N);
	for (int i = 0; i < Teams.Num(); ++i)
	{
		for (int j = 0; j < Teams.Num(); ++j)
		{
			if (i == j)
			{
				continue;
			}
			const FOpenSkillTeamRating TeamIRating = FOpenSkillModeling::GetTeamRating(Teams[i], 0);
			const FOpenSkillTeamRating TeamJRating = FOpenSkillModeling::GetTeamRating(Teams[j], 0);
			PairwiseProbabilities.Add(FOpenSkillStatistics::PhiMajor((TeamIRating.Mu - TeamJRating.Mu - DrawMargin) / FMath::Sqrt(N * BetaSquared + FMath::Square(TeamIRating.SigmaSq) + FMath::Square(TeamJRating.SigmaSq))));
		}
	}
	TArray<double> WinProbability;
	WinProbability.Reserve(PairwiseProbabilities.Num() / (N - 1));
	for (int i = 0; i < PairwiseProbabilities.Num(); i += N - 1)
	{
		double TeamProb = 0;
		for (int j = 0; j < N - 1; ++j)
		{
			TeamProb += PairwiseProbabilities[i + j];
		}
		WinProbability.Add(TeamProb / Denom);
	}
	for (int i = 0; i < WinProbability.Num(); ++i)
	{
		WinProbability[i] = FMath::Abs(WinProbability[i]);
	}
	TArray<int> Ranks = RankMinimum(WinProbability);
	const int MaxOrdinal = FMath::Max(Ranks);
	for (int i = 0; i < Ranks.Num(); ++i)
	{
		Ranks[i] = FMath::Abs(Ranks[i] - MaxOrdinal) + 1;
	}
	TArray<TTuple<int, double>> Results;
	Results.Reserve(Teams.Num());

	for (int i = 0; i < WinProbability.Num(); ++i)
	{
		Results.Emplace(Ranks[i], WinProbability[i]);
	}

	return Results;
}

double FOpenSkillUnrealModule::GetOrdinal(const FOpenSkillRating& Rating) const
{
	return Rating.Mu - Options.Z * Rating.Sigma;
}


TArray<TArray<FOpenSkillRating>> FOpenSkillUnrealModule::RateInternal(TArray<TArray<FOpenSkillRating>>&& Teams, TArray<int>&& Ranks, TArray<int>&& Weights) const
{
	if (Options.Tau > 0)
	{
		const double TauSquared = FMath::Square(Options.Tau);
		for (TArray<FOpenSkillRating>& Team : Teams)
		{
			for (FOpenSkillRating& Member : Team)
			{
				Member.Sigma = FMath::Sqrt(FMath::Square(Member.Sigma) + TauSquared);
			}
		}
	}

	TArray<TArray<FOpenSkillRating>> OrderedTeams;
	TArray<int> Tenet;
	FOpenSkillModeling::Unwind(Ranks, Teams, OrderedTeams, Tenet);
	// Sort ranks because the Teams list got sorted by the value of Ranks during Unwind.
	// Can be done in-place as it's not going to be used again against the original Teams list.
	Ranks.Sort();

	const TArray<TArray<FOpenSkillRating>> NewRatings = Options.Model(Teams, Ranks, Options);

	TArray<TArray<FOpenSkillRating>> ReorderedTeams;
	TArray<int> ReorderedTenet; // Unused.
	FOpenSkillModeling::Unwind(Tenet, NewRatings, ReorderedTeams, ReorderedTenet);

	if (Options.Tau > 0 && Options.PreventSigmaIncrease)
	{
		for (int i = 0; i < ReorderedTeams.Num(); ++i)
		{
			TArray<FOpenSkillRating>& Team = ReorderedTeams[i];
			for (int j = 0; j < Team.Num(); ++j)
			{
				FOpenSkillRating& Rating = Team[j];
				Rating.Sigma = FMath::Min(Rating.Sigma, Teams[i][j].Sigma);
			}
		}
	}
	return ReorderedTeams;
}

TArray<int> FOpenSkillUnrealModule::RankMinimum(const TArray<double>& A)
{
	const int N = A.Num();
	TArray<double> SortedA;
	TArray<int> Tenet;
	FOpenSkillModeling::UnwindByValue(A, SortedA, Tenet);

	double SumRanks = 0;
	int DuplicateCount = 0;

	TArray<int> Result;
	Result.AddZeroed(N);

	for (int i = 0; i < N; ++i)
	{
		SumRanks += i;
		DuplicateCount++;
		if (i == N - 1 || SortedA[i] != SortedA[i + 1])
		{
			for (int j = i - DuplicateCount + 1; j < i + 1; ++j)
			{
				Result[Tenet[j]] = i + 1 - DuplicateCount + 1;
			}
			SumRanks = 0;
			DuplicateCount = 0;
		}
	}
	return Result;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOpenSkillUnrealModule, OpenSkillUnreal)
