#pragma once

#include <array>
#include <format>
#include <optional>
#include <ranges>

namespace ExperisBowling {
    // Tracks score for a simple game of bowling.
    class Game {
    public:
        static constexpr unsigned FinalFrame = 10u;
        static constexpr unsigned FirstBonusFrame = FinalFrame + 1u;
        static constexpr unsigned SecondBonusFrame = FinalFrame + 2u;
        static constexpr unsigned MaxFrames = FinalFrame + 2u;
        static constexpr unsigned NumPins = 10u;

        static constexpr int SpareBonusRolls = 1;
        static constexpr int StrikeBonusRolls = 2;

        struct Frame {
            int                     bonusRolls          = 0;
            unsigned                currentScore        = 0u;
            bool                    isStrike            = false;
            bool                    isSpare             = false;
            std::optional<unsigned> pinsOnFirstRoll;
            std::optional<unsigned> pinsOnSecondRoll;
            unsigned                totalScore          = 0u;
        };

    private:
        unsigned                        currentRound = 0u;
        std::array<Frame, MaxFrames>    frames;

    public:
        // validates whether a particular roll is possible this round
        constexpr bool CheckRoll(unsigned pinCount, unsigned round) const {
            if (!frames[round].pinsOnFirstRoll) { // do we have a normal pin count for the first roll?
                return pinCount <= NumPins;
            }

            return frames[round].pinsOnFirstRoll.value() + pinCount <= NumPins;
        }

        // retrieves the index of the current game round
        constexpr unsigned GetCurrentRoundIndex() const {
            return currentRound;
        }

        // get any frame info by its index
        constexpr Frame const& GetFrame(size_t i) const {
            return frames[i];
        }

        // retrieves the current total score
        constexpr unsigned GetScore() const {
            for (Frame const& frame : frames | std::views::reverse) { // seek backwards through the frames for the score
                if (frame.totalScore > 0u) {
                    return frame.totalScore;
                }
            }

            return frames[0].currentScore;
        }

        // returns whether the bowling game has finished
        constexpr bool IsGameComplete() const {
            if (currentRound <= FinalFrame - 1u) {
                return false;
            }
            if (currentRound == FirstBonusFrame - 1u) {
                return !IsSpare(FinalFrame - 1u) && !IsStrike(FinalFrame - 1u);
            }
            if (currentRound == SecondBonusFrame - 1u) {
                return !(IsStrike(FinalFrame - 1u) && frames[FinalFrame - 1u].bonusRolls > 0u);
            }

            return true;
        }

        // tells the bowling game how many pins were knocked down by the latest roll
        constexpr std::optional<std::string> Roll(unsigned pinCount) {
            if (IsGameComplete()) {
                return "Game complete.";
            }
            if (pinCount > NumPins || !CheckRoll(pinCount, currentRound)) {
                return std::format("Invalid roll - Pin count: {}", pinCount);
            }

            // add in points for the final frame
            if (currentRound <= FinalFrame - 1u) {
                frames[currentRound].currentScore += pinCount;
            }

            // compute bonus points from two frames prior
            if (currentRound >= 2u && frames[currentRound - 2u].bonusRolls > 0) {
                Frame& priorFrame = frames[currentRound - 2u];
                priorFrame.currentScore += pinCount;
                priorFrame.bonusRolls -= 1;

                if (priorFrame.bonusRolls == 0) {
                    // are there any additional frames to sum with the current score?
                    if (currentRound >= 3u) { 
                        priorFrame.totalScore = frames[currentRound - 3u].totalScore + priorFrame.currentScore;
                    }
                    else { // no extra frames
                        priorFrame.totalScore = priorFrame.currentScore;
                    }
                }
            }

            // compute bonus points from the previous frame
            if (currentRound >= 1u && frames[currentRound - 1u].bonusRolls > 0) {
                Frame& prevFrame = frames[currentRound - 1u];
                prevFrame.currentScore += pinCount;
                prevFrame.bonusRolls -= 1;

                if (prevFrame.bonusRolls == 0) {
                    if (currentRound >= 2u) {
                        prevFrame.totalScore = frames[currentRound - 2u].totalScore + prevFrame.currentScore;
                    }
                    else {
                        prevFrame.totalScore = prevFrame.currentScore;
                    }
                }
            }

            if (!frames[currentRound].pinsOnFirstRoll) { // we need to set the score for the first roll
                frames[currentRound].pinsOnFirstRoll = pinCount;

                // did we get a strike? early out
                if (IsStrike(currentRound)) {
                    frames[currentRound].bonusRolls = StrikeBonusRolls;
                    frames[currentRound].isStrike = true;
                    currentRound++;

                    return std::nullopt;
                }

                // was the last round a spare? early out
                if (currentRound == FirstBonusFrame - 1u && IsSpare(FinalFrame - 1u)) {
                    currentRound++;

                    return std::nullopt;
                }

                // did we get double strikes?
                if (currentRound == SecondBonusFrame - 1u && IsStrike(FinalFrame - 1u) && IsStrike(FirstBonusFrame - 1u)) {
                    currentRound++;
                }
            }
            else { // we are on the second roll
                frames[currentRound].pinsOnSecondRoll = pinCount;

                if (IsSpare(currentRound)) { // account for spare bonus rolls
                    frames[currentRound].bonusRolls = SpareBonusRolls;
                    frames[currentRound].isSpare = true;
                }
                else { // average roll
                    // update the score
                    if (currentRound >= 1u) {
                        frames[currentRound].totalScore = frames[currentRound - 1u].totalScore + frames[currentRound].currentScore;
                    }
                    else {
                        frames[currentRound].totalScore = frames[currentRound].currentScore;
                    }
                }

                currentRound++;
            }

            return std::nullopt;
        }

        // calls Roll() to knock over any remaining pins
        constexpr std::optional<std::string> RollSpare() {
            Frame const& frame = frames[currentRound];
            if (!frame.pinsOnFirstRoll) {
                return "Invalid spare roll\n";
            }

            return Roll(NumPins - frame.pinsOnFirstRoll.value());
        }

        // calls Roll() to knock over all pins
        constexpr std::optional<std::string> RollStrike() {
            return Roll(NumPins);
        }

    private:
        constexpr bool IsSpare(unsigned round) const {
            return frames[round].pinsOnFirstRoll.value_or(0u) + frames[round].pinsOnSecondRoll.value_or(0u) == NumPins;
        }

        constexpr bool IsStrike(unsigned round) const { 
            return frames[round].pinsOnFirstRoll == NumPins; 
        }
    };
} // namespace ExperisBowling