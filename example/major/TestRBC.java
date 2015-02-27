import major.mutation.Config;

import java.util.List;

public class TestRBC{
    public static void main(String ... args){
        // Execute original version and gather coverage information
        Config.__M_NO=0;
        System.out.println("Original verdict: "+Main.run_rbc());

        System.out.println("\nCovered mutants:");
        // Get List of all covered mutants
        List<Integer> l = Config.getCoverageList();
        // Print all covered mutants
        for(Integer mut : l){
            System.out.print(mut+" ");
        }
        // Reset mutation coverage information
        Config.reset();
        
        // Iterate over covered mutants
        System.out.println("\n\nMutation analysis: ");
		  int n = 0;
        for(Integer mut : l){
            // Enable a certain mutant
            Config.__M_NO=mut;
				boolean b = Main.run_rbc();
				if (!b)
					n ++;
            System.out.println(mut+" - verdict: " + b);
        }
		  
		  System.out.println("\n\nNumber of mutants killed: " + n + " / " + l.size() + " (" + n*100.0/l.size() + "%)\n");
    }
}
