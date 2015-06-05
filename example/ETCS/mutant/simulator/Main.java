public class Main{
    public static boolean run_obu(){
        // Execute original version and gather coverage information
		  try{
			  ETCS.OBU obu = new ETCS.OBU();
			  System.out.println("\n-----------------------RUNING OBU-----------------------");
			  boolean b = obu.run("./obu.testcase.tc");
			  return b;
		  }catch (Exception ex){
			  System.out.println(ex);			  			  
			  //ex.printStackTrace(System.out);
		  }catch(StackOverflowError ex){
		  		System.out.println(ex);			  			  
		  }finally{
		  	  System.out.println("--------------------------------------------------------");
		  }
		  return false;
    }
	 
    public static boolean run_rbc(){
        // Execute original version and gather coverage information
		  try{
			  ETCS.RBC obu = new ETCS.RBC();
			  System.out.println("\n-----------------------RUNING RBC-----------------------");
			  boolean b = obu.run("./rbc.testcase.tc");
			  return b;
		  }catch (Exception ex){
			  System.out.println(ex);			  			  
			  //ex.printStackTrace(System.out);
		  }catch(StackOverflowError ex){
		  		System.out.println(ex);			  			  
		  }finally{
		  	  System.out.println("--------------------------------------------------------");
		  }
		  return false;
    }

	public static void main (String [] args){
		if (args.length != 1){
			System.out.println("Usage: ./xxx (OBU|RBC)");
			return;
		}
		
		boolean b = true;
		if (args[0] == "OBU")
			b = Main.run_obu();
		else
			b = Main.run_rbc();
		
		System.out.println("verdict: " + b);
		
	}
}