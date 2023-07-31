#pragma once
#include "Containers/Array.h"

struct FOpenSkillRating;
struct FOpenSkillTeamRating;
struct FOpenSkillOptions;

class OPENSKILLUNREAL_API FOpenSkillModeling
{
public:
	// Models, reference these as part of the Options struct
	static TArray<TArray<FOpenSkillRating>> PlackettLuce(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options);
	static TArray<TArray<FOpenSkillRating>> ThurstoneMostellerFull(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options);
	static TArray<TArray<FOpenSkillRating>> ThurstoneMostellerPartial(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options);
	static TArray<TArray<FOpenSkillRating>> BradleyTerryFull(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options);
	static TArray<TArray<FOpenSkillRating>> BradleyTerryPartial(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks, const FOpenSkillOptions& Options);

	// The default Gamma function, provide a different function if necessary in the Options struct
	static double DefaultGamma(const double C, const double K, const double Mu, const double SigmaSq, const TArray<FOpenSkillRating>& Team, const double Rank)
	{
		return FMath::Sqrt(SigmaSq) / C;
	}

	static double GetScore(double Q, double I);
	static TArray<int> GetRankings(const TArray<TArray<FOpenSkillRating>>& Teams, const TArray<int>& Ranks);
	static FOpenSkillTeamRating GetTeamRating(const TArray<FOpenSkillRating>& Team, const int Rank);
	static TArray<FOpenSkillTeamRating> GetTeamRatings(const TArray<TArray<FOpenSkillRating>>& Game, const TArray<int>& Ranks);

	template <typename ElementType>
	static TArray<TArray<ElementType>> GetLadderPairs(const TArray<ElementType>& Ranks);

	static double GetC(const TArray<FOpenSkillTeamRating>& TeamRatings, const FOpenSkillOptions& Options);
	static TArray<double> GetSumQ(const TArray<FOpenSkillTeamRating>& TeamRatings, double C);
	static TArray<double> GetA(const TArray<FOpenSkillTeamRating>& TeamRatings);

	template <typename ElementType>
	static void Unwind(const TArray<int>& Ranks, const TArray<ElementType>& SourceArray, TArray<ElementType>& SortedArray, TArray<int>& Tenet);

	template <typename ElementType>
	static void UnwindByValue(const TArray<ElementType>& SourceArray, TArray<ElementType>& SortedArray, TArray<int>& Tenet);
};

template <typename ElementType>
TArray<TArray<ElementType>> FOpenSkillModeling::GetLadderPairs(const TArray<ElementType>& Ranks)
{
	const int ZipSize = Ranks.Num() - 1;
	if (ZipSize < 1)
	{
		return {{}};
	}
	TArray<TArray<ElementType>> Result;
	for (int i = 0; i < Ranks.Num(); ++i)
	{
		int Left = i - 1;
		int Right = i + 1;
		if (i == 0)
		{
			Result.Add({Ranks[Right]});
		}
		else if (i == Ranks.Num() - 1)
		{
			Result.Add({Ranks[Left]});
		}
		else
		{
			Result.Add({Ranks[Left], Ranks[Right]});
		}
	}
	return Result;
}

template <typename ElementType>
void FOpenSkillModeling::Unwind(const TArray<int>& Ranks, const TArray<ElementType>& SourceArray, TArray<ElementType>& SortedArray, TArray<int>& Tenet)
{
	check(Ranks.Num() == SourceArray.Num())
	TArray<TTuple<int, int, ElementType>> Zipped;
	Zipped.Reserve(SourceArray.Num());

	for (int i = 0; i < SourceArray.Num(); ++i)
	{
		Zipped.Emplace(Ranks[i], i, SourceArray[i]);
	}
	Zipped.Sort([](const TTuple<int, int, ElementType>& Lhs, const TTuple<int, int, ElementType>& Rhs)
	{
		return Lhs.template Get<0>() < Rhs.template Get<0>();
	});
	SortedArray.Empty(SourceArray.Num());
	Tenet.Empty(Ranks.Num());
	for (const auto& Item : Zipped)
	{
		Tenet.Add(Item.template Get<1>());
		SortedArray.Add(Item.template Get<2>());
	}
}

template <typename ElementType>
void FOpenSkillModeling::UnwindByValue(const TArray<ElementType>& SourceArray, TArray<ElementType>& SortedArray, TArray<int>& Tenet)
{
	TArray<TTuple<int, ElementType>> Zipped;
	Zipped.Reserve(SourceArray.Num());

	for (int i = 0; i < SourceArray.Num(); ++i)
	{
		Zipped.Emplace(i, SourceArray[i]);
	}
	Zipped.Sort([](const TTuple<int, ElementType>& Lhs, const TTuple<int, ElementType>& Rhs)
	{
		return Lhs.template Get<1>() < Rhs.template Get<1>();
	});
	SortedArray.Empty(SourceArray.Num());
	Tenet.Empty(SourceArray.Num());
	for (const auto& Item : Zipped)
	{
		Tenet.Add(Item.template Get<0>());
		SortedArray.Add(Item.template Get<1>());
	}
}
