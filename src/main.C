/*
   Created by Huu Nghia NGUYEN
	nhnghia@me.com
   2014/03/31
*/

#include <fcntl.h>    /* For O_RDWR */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include <string>
#include <algorithm>

#include "model.h"
#include <signal.h>


static const char* DEADLINE_NAMES[3] = {"eager", "delayable", "lazy"};

/*
* convert the output of obj->Dump(stdout) to char*
*/
char* Dump(IfObject *obj){
	if (obj == NULL)
		return NULL;
	
	FILE *tmp = tmpfile();//create e temporary file
	obj->Dump(tmp);
	
	//read content of tmp file
	rewind(tmp);	//set postion of stream to the beginning
	char c;
	char buf[1024];
	int n=0;
	while ((c = getc(tmp)) != EOF){
		if (c == '"')
			buf[n++] = '\\';
		buf[n++] = c;
		if (n == 1024)
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
const char *getTarget (IfTransition *t){
	IfAction *act = t->GetTerminator();
	if (act == NULL)
		return "-";
	if (act->IsStop()){
		return "stop";
	}else if (act->IsNextstate()){
		IfNextstateAction *sa = (IfNextstateAction *) act;
		if (sa){
			char *s = Dump(sa);
			//s = nextstate s2; ==> extract "s2"
			int n = strlen(s);
			for (int i=10; i<n; i++)
				if (s[i] == ';')
					n = i;
			n -= 10;
			char *str = (char *) malloc(sizeof(char)*n);
			for (int i=0; i<n; i++)
				str[i] = s[i+10];
			return str;
		}
	}
	return "-";
}

int print_tran(IfState *src, IfTransition *tran){
	if (tran == NULL){
		return 0;
	}
	
	printf("\n  %s -> %s",
		src->GetName(),
		getTarget(tran)
	);
	
	printf(" [label=\"[%s] ",DEADLINE_NAMES[tran->GetDeadline()]);
	
	char *str;
	str = Dump(tran->GetProvided());
	if (str)
		printf("provided %s ", str);
	
	str = Dump(tran->GetWhen());
	if (str)
		printf("when %s ", str);
	
	str = Dump(tran->GetInput());
	if (str)
		printf("%s ", str);
	
	str = Dump(tran->GetBody());
	if (str){
		printf("/%s", str);
	}
	
	printf("\"];");
	
	return 1;
}

int print_state(IfState *s){
	if (s->IsStart()){
			printf("\n  %s[shape=doublecircle];", s->GetName());
	}
	IfList<IfTransition> *trans = s->GetOutTransitions();
	int n = trans->GetCount();
	
	for (int i=0; i<n; i++){
		print_tran(s, trans->GetAt(i));
	}
	return 1;
}

int print_proc(IfProcessEntity *proc){		
	printf("\n\n");
	printf("\ndigraph %s {", proc->GetName());
	printf("\n  stop [style=filled, fillcolor=red];");
	
	IfList<IfState> *states = proc->GetStates();
	
	int n = states->GetCount();
	for (int i=0; i<n; i++)
		print_state(states->GetAt(i));
	
	printf("\n}\n\n");
	
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
	rewind(des);
	return des;
}



int main(int argc, char * argv[]) {
  
  if (argc != 2) {
    fprintf(stderr, "\n Usage: %s fileName\n\n",argv[0]); 
    return 1;
  }
  
  FILE *file;
  file = fopen(argv[1], "r");
  file = copy(file);
  //file = fopen("ETCS_o.if", "r");
  IfSystemEntity *system = Load(file, NULL, 0);
  IfList<IfProcessEntity> *proc = system->GetProcesses();
  int n = proc->GetSize();
  for (int i=0; i<n; i++){
	  print_proc(proc->GetAt(i));
  }
  
  fclose(file);
  return 0; 
}


