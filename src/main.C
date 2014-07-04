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
#include <algorithm>

#include "model.h"
#include <signal.h>

using namespace std;

FILE *output;
char *fileName;
bool isOnlyState;

static const char* DEADLINE_NAMES[3] = {"eager", "delayable", "lazy"};

string replaceAll(const char *txt, char* c_from, char *c_to){
	string s_txt = string(txt);
	string s_from = string(c_from);
	string s_to = string(c_to);

	size_t start_pos = 0;
	start_pos = s_txt.find(s_from, start_pos);
	while(start_pos != s_txt.npos ) {
		s_txt.replace(start_pos, s_from.length(), s_to);
		start_pos += s_to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		start_pos = s_txt.find(s_from, start_pos);
	}
	return s_txt;
}

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
	fclose(tmp);	//close file stream ==> delete tmp file

	if (n==0)
		return "";

	char *str = (char*)malloc(sizeof(char) * n);
	for (int i=0; i<n; i++)
		str[i] = buf[i];
	str[n] = '\0';
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
			str[n] = '\0';
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
	
	fprintf(output, "\n  %s -> %s [label=\"", t1, t3);
	if (tran->GetDeadline() != 0)
		fprintf(output, " %s\\n",DEADLINE_NAMES[tran->GetDeadline()]);

	char *str;
	str = Dump(tran->GetProvided());
	if (str){
		fprintf(output, "[%s] ", str);
	}
	str = Dump(tran->GetWhen());
	if (str)
		fprintf(output, "[%s] ", str);
	
	str = Dump(tran->GetInput());
	if (str){
		//replace "input" by "?"

		fprintf(output, "%s ", replaceAll(str, "input", "?").c_str());
		//fprintf(output, "%s ", str);
	}
	if (isOnlyState){
		//fprintf(output, "body");
	}else{
		str = Dump(tran->GetBody());
		if (strlen(str)){
			string s = replaceAll(str, "task", "");
			fprintf(output, "\\n/%s", replaceAll(s.c_str(), "output", "!").c_str());
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
	//fprintf(output, "\n  stop [color=red];");
		
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
	long lSize;
	char * buffer;
	size_t result;

	// obtain file size:
	fseek(src, 0, SEEK_END);
	lSize = ftell(src);
	rewind (src);

	// allocate memory to contain the whole file:
	buffer = (char*) malloc(sizeof(char) * lSize);
	if (buffer == NULL) {
		fputs("Memory error", stderr);
		exit(2);
	}

	// copy the file into the buffer:
	result = fread(buffer, 1, lSize, src);
	if (result != lSize) {
		fputs("Reading error", stderr);
		exit(3);
	}
	string str = string(buffer);
	int d = str.find("system ");
	if (d != str.npos){
		d = str.find(';', d);	//find ; after system
		str.replace(d, 1, ";type integer = range 1..1;type pid = range 1..1;type boolean = range 0..1;type clock = range 0..1;");
	}
	FILE *des = tmpfile();
	fwrite(str.c_str(), sizeof(char), str.length(),des);
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


