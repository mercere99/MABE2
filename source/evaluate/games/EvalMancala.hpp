/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalMancala.hpp
 *  @brief MABE Evaluation module that has organisms play Mancala.
 */

#ifndef MABE_EVAL_MANCALA_HPP
#define MABE_EVAL_MANCALA_HPP

#include "emp/games/Mancala.hpp"

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalMancala : public Module {
  private:
    Collection target_collect;  // Which organisms should we evaluate?

    std::string input_trait;    // Name of trait to put input values.
    std::string output_trait;   // Name of trait to find output values.
    std::string score_trait;    // Trait to indicate game results.

    /// What type of opponent should we use?
    enum Opponent {
      RANDOM_MOVES,     // Opponent will always choose a random, legal move.
      AI,               // Opponent is a human-crafted AI.
      RANDOM_ORG,       // Opponent is a random organism from the population.
      UNKNOWN
    };

    Opponent opponent_type;

  public:
    EvalMancala(mabe::MABE & control,
                   const std::string & name="EvalMancala",
                   const std::string & desc="Evaluate organisms by having them play Mancala.",
                   const std::string & _itrait="input",
                   const std::string & _otrait="output",
                   const std::string & _strait="score")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , input_trait(_itrait)
      , output_trait(_otrait)
      , score_trait(_strait)
    {
      SetEvaluateMod(true);
    }
    ~EvalMancala() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(input_trait, "input_trait", "Into which trait should input values be placed?");
      LinkVar(output_trait, "output_trait", "Out of which trait should output values be read?");
      LinkVar(score_trait, "score_trait", "Which trait should we store success rating?");
      LinkMenu(opponent_type, "opponent_type", "Which type of opponent should organisms face?",
               RANDOM_MOVES, "random_moves", "Always choose a random, legal move.",
               AI, "ai", "Human supplied (but not very good) AI",
               RANDOM_ORG, "random_org", "Pick another random organism from collection."
      );
    }

    void SetupModule() override {
      AddOwnedTrait<emp::vector<double>>(input_trait, "Input values (curret board state)", emp::vector<double>({0.0}));
      AddRequiredTrait<emp::vector<double>>(output_trait); // Output values (move to make)
      AddOwnedTrait<double>(score_trait, "Play success", 0.0);
    }


    // Determine the next move of an organism.
    size_t EvalMove(emp::Mancala & game, Organism & org) {
      // Setup the hardware with proper inputs.
      org.GetVar<emp::vector<double>>(input_trait) = game.AsVectorInput(game.GetCurPlayer());

      // Run the code.
      org.GenerateOutput();

      emp::vector<double> results = org.GetVar<emp::vector<double>>(output_trait);

      // Determine the chosen move.
      size_t best_move = 0;
      for (int i = 1; i < 6; i++) {
        if (results[best_move] < results[i]) { best_move = i; }
      }

      return best_move;
    }

    // Determine the next move with human IO.
    size_t EvalMove(emp::Mancala & game, std::ostream & os=std::cout, std::istream & is=std::cin) {
      // Present the current board.
      game.Print();

      // Request a move from the human.
      char move;
      os << "Move?" << std::endl;
      is >> move;

      while (move < 'A' || move > 'F' || game.GetCurSide()[(size_t)(move-'A')] == 0) {
        os << "Invalid move! (choose a value 'A' to 'F')" <<  std::endl;
        is.clear();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        is >> move;
      }

      return (size_t) (move - 'A');
    }


    using mancala_ai_t = std::function< size_t(emp::Mancala & game) >;

    // Setup the fitness function for a whole game.
    double EvalGame(const mancala_ai_t & player0, const mancala_ai_t & player1,
                    bool cur_player=0, bool verbose=false) {
      emp::Mancala game(cur_player==0);
      size_t round = 0, errors = 0;
      while (game.IsDone() == false) {
        // Determine the current player and their move.
        auto & play_fun = (cur_player == 0) ? player0 : player1;
        size_t best_move = play_fun(game);

        if (verbose) {
          std::cout << "round = " << round++ << "   errors = " << errors << std::endl;
          game.Print();
          char move_sym = (char) ('A' + best_move);
          std::cout << "Move = " << move_sym;
          if (game.GetCurSide()[best_move] == 0) {
            std::cout << " (illegal!)";
          }
          std::cout << std::endl << std::endl;
        }

        // If the chosen move is illegal, shift through other options.
        while (game.GetCurSide()[best_move] == 0) {  // Cannot make a move into an empty pit!
          if (cur_player == 0) errors++;
          if (++best_move > 5) best_move = 0;
        }

        // Do the move and determine who goes next.
        bool go_again = game.DoMove(cur_player, best_move);
        if (!go_again) cur_player = !cur_player;
      }

      if (verbose) {
        std::cout << "Final scores -- A: " << game.ScoreA()
                  << "   B: " << game.ScoreB()
                  << std::endl;
      }

      return ((double) game.ScoreA()) - ((double) game.ScoreB()) - ((double) errors * 10.0);
    }

    mancala_ai_t ToOrgFun(mabe::Organism & org) {
      return [this,&org](emp::Mancala & game){ return EvalMove(game, org); };
    }

    // Wrapper for two Organisms competing
    double EvalGame(mabe::Organism & org0, mabe::Organism & org1, bool cur_player=0, bool verbose=false) {
      return EvalGame(ToOrgFun(org0), ToOrgFun(org1), cur_player, verbose);
    }

    // Wrapper for organism vs. random opponent.
    double EvalGame(mabe::Organism & org, emp::Random & random, bool cur_player=0, bool verbose=false) {
      mancala_ai_t rand_fun = [&random](emp::Mancala & game) {
        size_t move_id = random.GetUInt(6);
        while (!game.IsMoveValid(move_id)) move_id = random.GetUInt(6);
        return move_id;
      };
      return EvalGame(ToOrgFun(org), rand_fun, cur_player, verbose);
    }

    // Wrapper for organism vs. human
    double EvalGame(mabe::Organism & org, bool cur_player=0) {
      mancala_ai_t human_fun = [this](emp::Mancala & game){ return EvalMove(game, std::cout, std::cin); };
      return EvalGame(ToOrgFun(org), human_fun, cur_player, true);
    }


    void OnUpdate(size_t /* update */) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Determine the type of competitions to perform.
      // @CAO: For the moment, just doing a random opponent!!

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {
        double & score = org.GetVar<double>(score_trait);
        score = EvalGame(org, control.GetRandom());     // Start first.
        score += EvalGame(org, control.GetRandom(), 1); // Start second.
      }
    }
  };

  MABE_REGISTER_MODULE(EvalMancala, "Evaluate organisms on their ability to play Mancala.");
}

#endif
