// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OpenSkillTypes.h"
#include "OpenSkillOptions.h"
#include "Modules/ModuleManager.h"

class OPENSKILLUNREAL_API FOpenSkillUnrealModule : public IModuleInterface
{
public:
	static FOpenSkillUnrealModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FOpenSkillUnrealModule>("OpenSkillUnreal");
	}

	const FOpenSkillOptions& GetOptions() const
	{
		return Options;
	}

	void SetOptions(const FOpenSkillOptions& InOptions)
	{
		this->Options = InOptions;
	}

	/**
	 * @brief Rate applies match results to team members and returns their new skill scores.
	 * @param Teams The teams in place order (Teams[0] = 1st place, Teams[1] = 2nd place, etc.)
	 * @return The adjusted skill scoring based on placement.
	 */
	TArray<TArray<FOpenSkillRating>> RateByOrdinal(const TArray<TArray<FOpenSkillRating>>& Teams) const;
	/**
	 * @brief Rate applies match results to team members and returns their new skill scores.
	 * @param Teams An array of team and rank tuples. Multiple teams may have the same rank, lower values mean better placement in the ranking.
	 * @return The adjusted skill scoring based on placement.
	 */
	TArray<TArray<FOpenSkillRating>> RateByRank(const TArray<TTuple<TArray<FOpenSkillRating>, int>>& Teams) const;

	/**
	 * @brief Rate applies match results to team members and returns their new skill scores.
	 * @param Teams An array of team and game score tuples. Multiple teams may have the same score, higher values mean better placement in the ranking.
	 * @return The adjusted skill scoring based on placement.
	 */
	TArray<TArray<FOpenSkillRating>> RateByScore(const TArray<TTuple<TArray<FOpenSkillRating>, int>>& Teams) const;

	/**
	 * @brief PredictWin predicts how likely a match up against teams of one or more agents will go.
	 * @param Teams Two or more teams to evaluate.
	 * @return The probabilities of each team winning.
	 */
	TArray<double> PredictWin(const TArray<TArray<FOpenSkillRating>>& Teams) const;

	/**
	 * @brief Predict how likely a match up against teams of one or more agents will draw. This is unlikely to actually end in a draw unless game rules permit such, but can be used as a proxy of skill evenness.
	 * @param Teams Two or more teams to evaluate.
	 * @return  the probability of the teams drawing.
	 */
	double PredictDraw(const TArray<TArray<FOpenSkillRating>>& Teams) const;

	/**
	 * @brief Predict the shape of a match outcome.
	 * @param Teams Two or more teams to evaluate.
	 * @return A list of team ranks with their probabilities.
	 */
	TArray<TTuple<int, double>> PredictRank(const TArray<TArray<FOpenSkillRating>>& Teams) const;

	/**
	 * @brief Convert `mu` and `sigma` into a single value for sorting purposes.
	 * @param Rating The rating object.
	 * @return A number that represents a 1 dimensional value for a rating.
	 */
	double GetOrdinal(const FOpenSkillRating& Rating) const;

private:
	FOpenSkillOptions Options;

	TArray<TArray<FOpenSkillRating>> RateInternal(TArray<TArray<FOpenSkillRating>>&& Teams, TArray<int>&& Ranks, TArray<int>&& Weights) const;

	static TArray<int> RankMinimum(const TArray<double>& A);
};
