# Set of experiments to look at the trade-off between:
# * N = population size
# * T = number of test cases
# * G = number of generations
#
# The total number of evaluations in a run is N * T * G

set num_evals = 2560000
set in_filename = Tradeoffs.mabe
set mabe = ../../build/MABE

foreach mode (exploit struct_exploit explore diversity weak_diversity)
  mkdir -p $mode

  foreach num_traits (10 100 1000)
    set data_dir = $mode/T$num_traits
    mkdir -p $data_dir

    foreach pop_size (32 64 128 256 512 1024)
      set num_gens = `expr $num_evals / $pop_size`
      set treatment_config = (-s pop_size=$pop_size -s num_gens=$num_gens -s vals_org.N=$num_traits -s evaluator.diagnostic=\"$mode\")

      echo -----------------------------------
      echo num_traits = $num_traits
      echo pop_size = $pop_size
      echo num_gens = $num_gens

      foreach i (`seq 100 109`)
        set seed = $pop_size$num_traits$i
        set out_filename = $data_dir/$mode-T$num_traits-N$pop_size-G$num_gens-$i.csv
        set run_config = (-s random_seed=$seed -s out_file.filename=\"$out_filename\")
        set run_settings = (-f $in_filename $treatment_config $run_config)
        echo =====
        echo Generating: $out_filename
        echo Random Seed: $seed

        echo $mabe $run_settings
        $mabe $run_settings
      end

    end
  end
end 
