/*
   Created by Huu Nghia NGUYEN
	nhnghia@me.com
   2014/03/31
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <string>

#include "model.h"
#include <signal.h>

FILE *output;
char *fileName;
bool isOnlyState;

static const char* DEADLINE_NAMES[3] = {"eager", "delayable", "lazy"};

/*
* convert the output of obj->Dump(stdout) to char*
*/
char* Dump(IfObject *obj){
	if (obj == NULL)
		return NULL;
	
	FILE *tmp = tmpfile();//create e temporary file
	obj->Dump(tmp);
	fflush(tmp);
	
	//read content of tmp file
	rewind(tmp);	//set postion of stream to the beginning
	char c;
	char buf[1024] = {0};
	int n=0;
	while ((c = getc(tmp)) != EOF){
		if (c == '"')
			buf[n++] = '\\';
		buf[n++] = c;
		if (n == 1022)
			break;
	}
	char *str = (char*)malloc(sizeof(char) * n);
	for (int i=0; i<n; i++)
		str[i] = buf[i];
	fclose(tmp);	//close file stream ==> delete tmp file
	return str;
}

/*
* Target of a transition is either a state (IfNextstateAction) or a stop (IfStopAction)
*/
const char *getTarget (IfTransition *tran){
	IfAction *act = tran->GetTerminator();
	if (act == NULL)
		return "-";
	if (act->IsStop()){
		return "stop";
	}else if (act->IsNextstate()){
		IfNextstateAction *sa = (IfNextstateAction *) act;
		if (sa){
			char *s = Dump(sa);
			//s = nextstate s2; ==> extract "s2"
			//printf("\n%s", s);
			int n = strlen(s);
			for (int i=10; i<n; i++)
				if (s[i] == ';' || s[i] == ' ' || s[i] == '#'){
					n = i;
					break;
				}
			n -= 10;
			char *str = (char *) malloc(sizeof(char)*n);
			for (int i=0; i<n; i++)
				str[i] = s[i+10];
			//printf("===%s==", str);
			return str;
		}
	}
	return "_";
}

int print_tran(IfState *src, IfTransition *tran){
	if (tran == NULL){
		return 0;
	}
	
	char *t1 = src->GetName();
	const char* t2 = getTarget(tran);
	char *t3;
	if (strcmp(t2, "-") == 0)
		t3 = t1;
	else{
		int n = strlen(t2);
		t3 = new char[n];
		for (int i=0; i<n; i++)
			t3[i] = t2[i];
	}
	
	fprintf(output, "\n  %s -> %s", t1, t3);
	
	fprintf(output, " [label=\"[%s]\\n",DEADLINE_NAMES[tran->GetDeadline()]);

	char *str;
	str = Dump(tran->GetProvided());
	if (str)
		fprintf(output, "provided %s ", str);

	str = Dump(tran->GetWhen());
	if (str)
		fprintf(output, "when %s ", str);
	
	str = Dump(tran->GetInput());
	if (str)
		fprintf(output, "%s ", str);
	
	if (isOnlyState){
		//fprintf(output, "body");
	}else{
		str = Dump(tran->GetBody());
		if (str){
			fprintf(output, "\\n/%s", str);
		}
	}
	
	fprintf(output, "\"];");
	
	return 1;
}

int print_state(IfState *s){
	if (s->IsStart()){
			fprintf(output, "\n  %s[shape=doublecircle];", s->GetName());
	}

	IfList<IfTransition> *trans = s->GetOutTransitions();
	int n = trans->GetCount();
	
	for (int i=0; i<n; i++){
		print_tran(s, trans->GetAt(i));
	}
	return 1;
}

int print_proc(IfProcessEntity *proc){		
	const char *procName = proc->GetName();
	int len = strlen(fileName) + strlen(procName) + 5;
	char *str = new char[len];

	sprintf(str, "%s.%s.dot", fileName, procName);
	
	printf("\n  Write the process [%s] to file [%s]", procName, str);

	output = fopen(str, "w");
	if (output == NULL){
		printf("\n  Error: cannot create file [%s]\n", str);
		exit(0);
	}
	fprintf(output, "digraph %s {", procName);
	fprintf(output, "\n  stop [color=red];");
		
	IfList<IfState> *states = proc->GetStates();
	
	int n = states->GetCount();
	for (int i=0; i<n; i++){
		print_state(states->GetAt(i));
	}
	fprintf(output, "\n}\n\n");

	return 1;
}



/**
* truck technique
* I get error when standard types are not declared. I don't understand why
* So I copy the IF file to a temporary file, then insert the definition of these types
*  type integer = range 1..1;
*  type pid = range 1..1;
*  type boolean = range 0..1;
*  type clock = range 0..1;
*/

FILE* copy(FILE *src){
	FILE *des = tmpfile();
	char *sys = "system ";
	char buf[7];
	
	int nread;
	char c;
	bool inserted = false;
	
	while ((nread = fread(buf, 1, sizeof buf, src)) > 0){
		fwrite(buf, 1, nread, des);
		if (!inserted){
			//this may not happen when the file starts by a comment or sth different with "system"
			if (strcmp(buf,sys) == 0){		
				while ((c = getc(src)) != EOF){
					fprintf(des, "%c", c);
					if (c == ';'){
						inserted = true;
						fprintf(des, "type integer = range 1..1;type pid = range 1..1;type boolean = range 0..1;type clock = range 0..1;");
						break;
					}
				}
			}
		}
	}
	fflush(des);
	rewind(des);
	return des;
}



int main(int argc, char * argv[]) {
  
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "\n Usage: %s fileName [state]",argv[0]); 
	 fprintf(stderr, "\n         [state] : do not draw body of transition labels");
	 fprintf(stderr, "\n\n");
    return 0;
  }
  
  FILE *file;
  file = fopen(argv[1], "r");
  if (file == NULL){
	  fprintf(stderr, "Cannot open file: %s", argv[1]);
	  return 0;
  }

  fileName = argv[1];
  //remove .if at the end of fileName
  fileName[strlen(fileName) - 3] = '\0';
  isOnlyState = (argc == 3);


  file = copy(file);
 

  IfSystemEntity *system = Load(file, NULL, 0);
  IfList<IfProcessEntity> *proc = system->GetProcesses();
  int n = proc->GetCount();
  for (int i=0; i<n; i++){
	  print_proc(proc->GetAt(i));
  }
  
  fclose(file);
  printf("\n\n");
  return 1; 
}


