/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
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
    OwnedTrait<emp::vector<double>> input_trait {this, "input", "Input values (current board state)"};
    RequiredTrait<emp::vector<double>> output_trait {this, "output"}; // Output values (move to make)
    OwnedTrait<double> scoreA_trait {this, "scoreA", "Score for this player"};
    OwnedTrait<double> scoreB_trait {this, "scoreB", "Score for opponent"};
    OwnedTrait<double> error_trait {this, "num_errors", "Number of illegal moves attempted"};
    OwnedTrait<double> fitness_trait {this, "fitness", "Combined success rating"};

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
                emp::String name="EvalMancala",
                emp::String desc="Evaluate organisms by having them play Mancala.")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalMancala() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalMancala & mod, Collection orgs) { return mod.Evaluate(orgs); },
                             "Evaluate organism's ability to play the game Mancala.");
      info.AddMemberFunction("TRACE",
                             [](EvalMancala & mod, Collection orgs, const emp::String & filename) {
                                std::ofstream file(filename);
                                mod.TraceEval(orgs, file);
                                return orgs.GetSize();
                              },
                             "Trace the Mancala game-play during evaluation.");
    }

    void SetupConfig() override {
      LinkMenu(opponent_type, "opponent_type", "Which type of opponent should organisms face?",
               RANDOM_MOVES, "random", "Always choose a random, legal move.",
               AI, "ai", "Human supplied (but not very good) AI",
               RANDOM_ORG, "random_org", "Pick another random organism from collection."
      );
    }

    // Determine the next move of an organism.
    size_t EvalMove(emp::Mancala & game, Organism & org) {
      // Setup the hardware with proper inputs.
      input_trait(org) = game.AsVectorInput(game.GetCurPlayer());

      // Run the code.
      org.GenerateOutput();

      emp::vector<double> results = output_trait(org);

      // Determine the chosen move.
      size_t best_move = 0;
      const size_t move_cap = std::min<size_t>(results.size(), 6);
      for (size_t i = 1; i < move_cap; i++) {
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

    /// A uniform function specification that takes a game state and returns a move to make.
    using mancala_ai_t = std::function< size_t(emp::Mancala & game) >;

    /// Information about the results of a match.
    struct Results {
      size_t scoreA = 0;
      size_t scoreB = 0;
      size_t num_errors = 0;

      double CalcFitness() const {
        return ((double) scoreA) - ((double) scoreB) - ((double) num_errors * 10.0);
      }
    };

    /// Evaluate a game between two functions that each take the game state as input and return
    /// their next move as output.
    /// @param player0 The function to be evaluated
    /// @param player1 The function to test against
    /// @param cur_player Which player should make the first move?  (default=0, the organism)
    /// @param verbose Should we print out extra output? (default=false)
    /// @param os Output stream for any extra ouput. (default=cout)
    Results EvalGame(const mancala_ai_t & player0, const mancala_ai_t & player1, bool cur_player=0,
                    bool verbose=false, std::ostream & os=std::cout) {
      emp::Mancala game(cur_player==0);
      size_t round = 0, errors = 0;
      while (game.IsDone() == false) {
        // Determine the current player and their move.
        auto & play_fun = (cur_player == 0) ? player0 : player1;
        size_t best_move = play_fun(game);

        if (verbose) {
          os << "round = " << round++ << "   errors = " << errors << std::endl;
          game.Print(os);
          char move_sym = (char) ('A' + best_move);
          os << "Move = " << move_sym;
          if (game.GetCurSide()[best_move] == 0) {
            os << " (illegal!)";
          }
          os << std::endl << std::endl;
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
        os << "Final scores -- A: " << game.ScoreA()
                  << "   B: " << game.ScoreB()
                  << std::endl;
      }

      return Results{ game.ScoreA(), game.ScoreB(), errors };
    }

    /// Convert an organism into a uniform function that can be plugged into Mancala.
    mancala_ai_t ToOrgFun(mabe::Organism & org) {
      return [this,&org](emp::Mancala & game){ return EvalMove(game, org); };
    }

    /// Evaluate a game: Organism vs. Organism.
    /// @param org0 The organism to be evaluated
    /// @param org1 The organism to test against
    /// @param start_player Which player should make the first move?  (default=0, the organism)
    /// @param verbose Should we print out extra output? (default=false)
    /// @param os Output stream for any extra ouput. (default=cout)
    Results EvalGame(mabe::Organism & org0, mabe::Organism & org1, bool start_player=0,
                    bool verbose=false, std::ostream & os=std::cout) {
      return EvalGame(ToOrgFun(org0), ToOrgFun(org1), start_player, verbose, os);
    }

    /// Evaluate a game: Organism vs. random opponent.
    /// @param org The organism to be evaluated
    /// @param random The random number generator to use for opponent moves.
    /// @param start_player Which player should make the first move?  (default=0, the organism)
    /// @param verbose Should we print out extra output? (default=false)
    /// @param os Output stream for any extra ouput. (default=cout)
    Results EvalGame(mabe::Organism & org, emp::Random & random, bool start_player=0,
                    bool verbose=false, std::ostream & os=std::cout) {
      mancala_ai_t rand_fun = [&random](emp::Mancala & game) {
        size_t move_id = random.GetUInt(6);
        while (!game.IsMoveValid(move_id)) move_id = random.GetUInt(6);
        return move_id;
      };
      return EvalGame(ToOrgFun(org), rand_fun, start_player, verbose, os);
    }

    /// Evaluate a game: Organism vs. human opponent.
    /// @param org The organism to be evaluated
    /// @param start_player Which player should make the first move?  (default=0, the organism)
    Results EvalGame(mabe::Organism & org, bool start_player=0) {
      mancala_ai_t human_fun = [this](emp::Mancala & game){
        return EvalMove(game, std::cout, std::cin);
      };
      return EvalGame(ToOrgFun(org), human_fun, start_player, true);
    }

    /// Trace the evaluation of an organism, sending output to a specified stream.
    void TraceEval(Organism & org, std::ostream & os) {
      EvalGame(org, control.GetRandom(), 0, true, os);
    }

    /// Trace the evaluation of a collection of organisms.
    void TraceEval(Collection & orgs, std::ostream & os) {
      for (Organism & org : orgs) {
        EvalGame(org, control.GetRandom(), 0, true, os);
      }
    }

    double Evaluate(const Collection & orgs) {
      // Determine the type of competitions to perform.
      // ==> @CAO: For the moment, just doing a random opponent!!

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( orgs.GetAlive() );

      control.Verbose(" - ", alive_collect.GetSize(), " organisms found.");

      size_t org_count = 0;
      double max_fitness = 0.0;
      for (Organism & org : alive_collect) {
        control.Verbose("...eval org #", org_count++);
        double & scoreA = scoreA_trait(org);
        double & scoreB = scoreB_trait(org);
        double & num_errors = error_trait(org);
        double & fitness = fitness_trait(org);
        Results results = EvalGame(org, control.GetRandom());  // Start first.
        scoreA = results.scoreA;
        scoreB = results.scoreB;
        num_errors = results.num_errors;
        fitness = results.CalcFitness();

        results = EvalGame(org, control.GetRandom(), 1);  // Start second.
        scoreA += results.scoreA;
        scoreB += results.scoreB;
        num_errors += results.num_errors;
        fitness += results.CalcFitness();

        if (fitness > max_fitness) max_fitness = fitness;
      }

      return max_fitness;
    }

    // If a population is provided to Evaluate, first convert it to a Collection.
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    // If a string is provided to Evaluate, convert it to a Collection.
    double Evaluate(const emp::String & in) { return Evaluate( control.ToCollection(in) ); }
  };

  MABE_REGISTER_MODULE(EvalMancala, "Evaluate organisms on their ability to play Mancala.");
}

#endif
