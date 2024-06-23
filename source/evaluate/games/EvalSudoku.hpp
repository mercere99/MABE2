/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file  EvalSudoku.hpp
 *  @brief Evaluation module to determine the quality of a sudoku board.
 * 
 *  Sudoku solving traits that can be selected:
 *    profile_length : How many distinct solving steps are there?
 *    profile diversity : How many types of moves are needed?
 *    ? neg of max move count : Fewest move options
 *  -> Only if solved:
 *    number of empty cells on starting grid
 *    count of each move type : how many times does the relevant move need to be used?
 *    ? is move type a bottleneck? : 0/1 Is there ever only one option for move?
 */

#ifndef MABE_EVAL_SUDOKU_H
#define MABE_EVAL_SUDOKU_H

#include "emp/games/SudokuAnalyzer.hpp"
#include "emp/tools/String.hpp"

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalSudoku : public Module {
  private:
    emp::SudokuAnalyzer analyzer;

    emp::String target_filename;           // File to load with a target board.
    emp::array<uint8_t, 81> target_board;  // Fixed positions to target for the final board.

    RequiredMultiTrait<size_t> states_trait{this, "states",    "Starting states for Sudoku board", emp::SudokuAnalyzer::GetNumCells()};

    OwnedTrait<double> solve_trait{this,      "solvable",    "Is this game solvable? 0/1"};
    OwnedTrait<double> length_trait{this,     "puz_length",  "How long is the solving profile for this game?"};
    OwnedTrait<double> diverse_trait{this,    "puz_variety", "number of types of moves are used in solution?"};
    OwnedTrait<double> empty_trait{this,      "puz_empty",   "Empty cell count at start of a SOLVABLE game (0=unsolvable)"};
    OwnedMultiTrait<double> count_trait{this, "move_counts", "number of times is a move type needed (for each move)?", emp::SudokuAnalyzer::GetNumMoveTypes()};
    OwnedTrait<double> score_trait{this,      "score",       "overall score for sudoku board"};

    OwnedTrait<double> match_trait{this,      "puz_match",   "How well does this puzzle match a target?"};


  public:
    EvalSudoku(mabe::MABE & control,
               emp::String name="EvalSudoku",
               emp::String desc="Evaluate states for the qualities of the Sudoku game they produce.")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalSudoku() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalSudoku & mod, Collection list) { return mod.Evaluate(list); },
                             "Evaluate the scores for one or more Sudoku boards.");
      info.AddMemberFunction("PRINT",
                             [](EvalSudoku & mod, Collection list) { return mod.Print(list); },
                             "Print one or more Sudoku boards.");
    }

    void SetupConfig() override {
      LinkVar(target_filename, "target_file", "File with info about any cell states to target at end.");
      // No other variables to link in to the configuration (e.g., N and K for NK)
    }

    void SetupModule() override {
      // Make sure we haven't messed up the number of solution functions.  (@CAO - make better!)
      emp_assert(analyzer.GetNumSolveFuns() == emp::SudokuAnalyzer::GetNumMoveTypes());

      // Load in a target board, if any.
      if (target_filename.size()) {
        target_board = analyzer.LoadToArray(target_filename);
      }
    }

    template <typename T>
    uint8_t TestTarget(const T & values) {
      if (target_filename.size() == 0) return 0; // Nothing to target.
      uint8_t count = 0;
      for (size_t i = 0; i < 81; ++i) {
        if (target_board[0] && target_board[i] == values[i]) ++count;
      }
      return count;
    }

    double Evaluate(Collection orgs) {
      emp_assert(control.GetNumPopulations() >= 1);

      // Evaluate each organism.
      double max_score = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its genome ready for us to access.
        org.GenerateOutput();

        // Load the Sudoku board.
        std::span<size_t> genome = states_trait(org); // GetTrait<size_t>(SharedData().genome_name, SharedData().genome_size);
        if (analyzer.Load(genome)) {
          auto profile = analyzer.CalcProfile();
          bool solved = analyzer.IsSolved();

          // Set stats for this organism.
          solve_trait(org) = solved;
          length_trait(org) = profile.size();
          diverse_trait(org) = profile.CountTypes();
          empty_trait(org) = solved ? std::count(genome.begin(), genome.end(), 0) : 0.0;
          match_trait(org) = static_cast<double>(TestTarget(analyzer.GetValues()));

          for (size_t move_id = 0; move_id < emp::SudokuAnalyzer::GetNumMoveTypes(); ++move_id) {
            count_trait(org)[move_id] = solved ? profile.CountMoves(move_id) : 0.0;
          }

          double score = profile.CalcScore() + (solved ? 1000.0 : 0) + TestTarget(analyzer.GetValues())*75.0 - TestTarget(genome)*25;
          score_trait(org) = score;

          if (score > max_score || !max_org) {
            max_score = score;
            max_org = &org;
          }
        }

        // Otherwise, if load failed, indicate so in traits (illegal starting position!)
        else {
          solve_trait(org) = false;
          length_trait(org) = 0.0;
          diverse_trait(org) = 0.0;
          empty_trait(org) = 0.0;
          match_trait(org) = 0.0;
          for (size_t move_id = 0; move_id < emp::SudokuAnalyzer::GetNumMoveTypes(); ++move_id) {
            count_trait(org)[move_id] = 0.0;
          }
          score_trait(org) = 0.0;
        }
      }

      // std::cout << "Max Sudoku score: " << score_trait.GetName() << " = " << max_score << std::endl;
      return max_score;
    }

    double Print(Collection orgs) {
      // Print each organism.
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its genome ready for us to access.
        org.GenerateOutput();

        // Load the Sudoku board.
        std::span<size_t> genome = states_trait(org);
        if (analyzer.Load(genome)) {
          analyzer.Print();
          auto profile = analyzer.CalcProfile();
          bool solved = profile.IsSolved();

          // // Set stats for this organism.
          // solve_trait(org) = solved;
          // length_trait(org) = profile.size();
          // diverse_trait(org) = profile.CountTypes();
          // empty_trait(org) = solved ? std::count(genome.begin(), genome.end(), 0) : 0.0;

          // for (size_t move_id = 0; move_id < emp::SudokuAnalyzer::GetNumMoveTypes(); ++move_id) {
          //   count_trait(org)[move_id] = solved ? profile.CountMoves(move_id) : 0.0;
          // }

          double score = profile.CalcScore() + (solved ? 1000.0 : 0) + TestTarget(analyzer.GetValues())*75.0 - TestTarget(genome)*25;
          std::cout << "SCORE = " << score
            << "  solvable=" << solved
            << "  solve length=" << profile.size()
            << "  solve variety=" << profile.CountTypes()
            << "  solve empty=" << std::count(genome.begin(), genome.end(), 0)
            << "  match = " << TestTarget(analyzer.GetValues())
            << "  direct_match = " << TestTarget(genome)
            << std::endl;
        }

        // Otherwise, if load failed, indicate as much!
        else {
          std::cout << "ILLEGAL BOARD - CANNOT LOAD!" << std::endl;
        }
      }

      return 1.0;
    }
  };

  MABE_REGISTER_MODULE(EvalSudoku, "Evaluate bitstrings by counting ones (or zeros).");
}

#endif
