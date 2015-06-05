numOrdPurposes = 0;
numPurposes = 1;

int i;
for (i=0; i < numPurposes;i++) 
{
	purposes[i].status = false;
	purposes[i].visited = false;      	
	purposes[i].process = "{OBU}0";  
	purposes[i].source = NULL;      	
	purposes[i].target = NULL;  			
	purposes[i].numBoundClocks = 0;
	purposes[i].numActiveClocks = 0;
	purposes[i].numSignals = 0;
	purposes[i].numVariables = 0;
	purposes[i].depth = -1;
}

//purposes[0].source = ""; 


//purposes[0].numVariables = 1;

//m.speed
//purposes[0].variables[0] = {"m.speed", "80"};
//v
//purposes[0].variables[0] = {"v", "0"};


//purposes[0].numBoundClocks = 1;
//ClockData c = ;
purposes[0].clocks[0] = {"c", 60};

purposes[0].numSignals = 1;

purposes[0].signals[0] = {"EBcmd", "output", "{0}"};
