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

using namespace std;


class State: public IfNextstateAction{
	public:
	char *getStateName(){
		char *str = (char *) malloc(strlen(m_pStateName));
		strcpy(str, m_pStateName);
		return str;
	}
};

FILE *output;
char *output_folder;
bool isOnlyState;
int tranID;

//system name
const char *sysName;

//IfRangeType ==> int
char *range_types[20];
int range_type_number = 0;

char* replaceAll(const char *txt, char* c_from, char *c_to){
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
	return strdup(s_txt.c_str());
}

/*
* convert the output of obj->Dump(FILE) to char*
*/
char* Dump(IfObject *obj){
	if (obj == NULL)
		return "";
	
	FILE *tmp = tmpfile();//create e temporary file

	obj->Dump(tmp);
	fflush(tmp);
	
	//read content of tmp file
	rewind(tmp);	//set postion of stream to the beginning
	char c;
	char buf[1024] = {0};
	int n=0;
	while ((c = getc(tmp)) != EOF){
		if (c == ';'){

		}else{
			if (c == '"')
				buf[n++] = '\\';
			buf[n++] = c;
			if (n == 1023)
				break;
		}
	}
	fclose(tmp);	//close file stream ==> delete tmp file

	if (n==0)
		return "";

	char *str = (char*)malloc(sizeof(char) * n);
	strcpy(str, buf);
	return str;
}

/**
 * Print actions (task, set, reset) in body of a transition
 * @param body
 * @return
 */
void printBody(const char *space, IfBlockStatement *body){
	IfList<IfBasicStatement> *statements = body->GetStatements();
	if (statements == NULL)
		return;
	int n=statements->GetSize();
	for (int i=0; i<n; i++){
		IfBasicStatement *s = statements->GetAt(i);
		if (s == NULL)
			continue;

		if (s->IsAction()){
			IfActionStatement *as = (IfActionStatement*) s;
			IfAction *act = as->GetAction();
			if (act->IsTask()){
				IfTaskAction *out = (IfTaskAction *) act;
				//output to file
				fprintf(output, "%s%s = %s;", space, Dump(out->GetLhs()), Dump(out->GetRhs()));

			}
			//clock
			else if (act->IsSet()){
				IfSetAction *out = (IfSetAction *) act;
				fprintf(output, "%s%s = %s; //set clock", space, Dump(out->GetLhs()), Dump(out->GetRhs()));
			}
			else if (act->IsReset()){
				IfResetAction *out = (IfResetAction *) act;
				fprintf(output, "%s%s = 0; //reset clock", space, Dump(out->GetLhs()));
			}
			else if (act->IsCall()){
				IfCallAction *ca = (IfCallAction *) act;
				fprintf(output, "%s%s = %s(", space, Dump(ca->GetLhs()), ca->GetProcedure()->GetName());
				IfList<IfExpression> *exp = ca->GetParameters();
				int n = exp->GetSize();
				for (int i=0; i<n; i++){
					IfExpression *e = exp->GetAt(i);
					if (e == NULL)
						continue;
					if (i == 0)
						fprintf(output, "%s", Dump(e));
					else
						fprintf(output, ", %s", Dump(e));
				}
				fprintf(output, ");");
			}
			else if (act->IsOutput()){
				IfInputAction *in = (IfInputAction *) act;

				fprintf(output, "%soutput(\"%s\"", space, in->GetSignal()->GetName());
				IfList<IfExpression> *vars = in->GetParameters();
				int n = vars->GetSize();
				for (int i=0; i<n; i++){
					IfExpression *v = vars->GetAt(i);
					if (v == NULL)
						continue;
					fprintf(output, ", %s", Dump(v));
				}
				fprintf(output, ");");
			}
		}
		else if (s->IsBlock()){
			//if, while, ..
			IfBlockStatement *block = (IfBlockStatement*) s;

			printBody("", block);
		}
	}
}
/**
 * Get an output from body of a transition
 * @param body
 * @return
 */
char *getOutput(IfBlockStatement *body){
	IfList<IfBasicStatement> *statements = body->GetStatements();
	if (statements == NULL)
		return "";
	int n=statements->GetSize();
	for (int i=0; i<n; i++){
		IfBasicStatement *s = statements->GetAt(i);
		if (s == NULL)
			return "";

		if (s->IsAction()){
			IfActionStatement *as = (IfActionStatement*) s;
			IfAction *act = as->GetAction();
			if (act->IsOutput()){
				IfOutputAction *out = (IfOutputAction *) act;
				char *str = Dump(out); //e.g., output sdt(m, b) via ({tr}0);

				return str;
			}
		}else if (s->IsBlock()){
			//if, while, ..
			IfBlockStatement *block = (IfBlockStatement*) s;
			char *str = getOutput(block);
			if (strcmp(str, ""))
				return str;
		}
	}
	return "";
}

/**
 * get java type from IfType
 * @param type
 * @return
 */
char * get_type(IfType *type){
	if (type->IsString())
		return "String";
	else if (type->IsBasic()){
		//BOOLEAN = 0, INTEGER, REAL, CLOCK, PID, VOID
		char * data[6] = {"bool", "int", "float", "Clock", "", "void"};
		IfBasicType *bt = (IfBasicType *) type;
		return data[bt->GetKind()];
	}

	//integer
	char *name = type->GetName();
	if (strcmp(name, "integer") == 0)
		return "int";
	if (strcmp(name, "clock") == 0)
			return "Integer";
	if (strcmp(name, "pid") == 0)
			return strdup(sysName);
	if (strcmp(name, "bool") == 0)
				return "boolean";
	//range
	for (int i=0; i<range_type_number; i++){
		if (strcmp(name, range_types[i]) == 0)
			return "int";
	}
	return name;
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
			State *state = (State *) sa;
			return state->getStateName();
		}
	}
	return "_";
}

int print_input(const char *space, IfAction *input){
	if (input == NULL)
		return 0;

	if (input->IsInput()){
		IfInputAction *in = (IfInputAction *) input;

		fprintf(output, "%sinput(\"%s\"", space, in->GetSignal()->GetName());
		IfList<IfExpression> *vars = in->GetParameters();
		int n = vars->GetSize();
		for (int i=0; i<n; i++){
			IfExpression *v = vars->GetAt(i);
			if (v == NULL)
				continue;
			fprintf(output, ", %s", Dump(v));
		}
		fprintf(output, ");");
	}
	return 1;
}


int print_tran(IfState *src, IfTransition *tran){
	if (tran == NULL){
		return 0;
	}
	
	char *t1 = src->GetName();
	const char* t2 = getTarget(tran);
	char *dst;
	if (strcmp(t2, "-") == 0)
		dst = t1;
	else{
		int n = strlen(t2);
		dst = (char *) malloc(sizeof(char) *n);
		strcpy(dst, t2);
	}
	
	char *inStr = Dump(tran->GetInput());
	inStr = replaceAll(inStr, "input ", "");
	char *outStr = getOutput(tran->GetBody());

	fprintf(output, "\n		//transition id=\"tr%d\" dst=\"%s\" input=\"%s\" output=\"%s\"",
			(++tranID),
			dst,
			inStr,
			outStr);
	
	bool isIf = false;
	char *whenStr = Dump(tran->GetWhen());
	if (strcmp(whenStr, "")){
		fprintf(output, "\n		if (%s) {", replaceAll(whenStr, " = ", "=="));
		isIf = true;
	}
	char *providedStr = Dump(tran->GetProvided());

	if (strcmp(providedStr, "")){
		fprintf(output, "\n		if (%s) {", replaceAll(providedStr, " = ", "=="));
		isIf = true;
	}

	char *space = "\n		";
	if (isIf)
		space = "\n			";

	print_input(space, tran->GetInput());
	printBody(space, tran->GetBody());
	fprintf(output, "%snextState(\"%s\");", space, dst);

	if (isIf)
		fprintf(output, "\n		}");

	return 1;
}


int print_state(IfState *s){
	if (s->IsStart()){
			fprintf(output, "\n\n	public void state_init(){");
	}else
		fprintf(output, "\n\n	public void state_%s(){", s->GetName());

	IfList<IfTransition> *trans = s->GetOutTransitions();
	int n = trans->GetCount();
	
	for (int i=0; i<n; i++){
		print_tran(s, trans->GetAt(i));
	}

	fprintf(output, "\n	}");
	return 1;
}

int print_variables(char *space, IfList<IfVariable> *variables){
	int n = variables->GetCount();

	for (int i=0; i<n; i++){
		IfVariable *var = variables->GetAt(i);
		//if (var->GetType()->IsBasic())
		{
			char *val = Dump(var->GetInitializer());
			if (val == NULL || strlen(val) == 0)
				fprintf(output, "\n%spublic %s %s;", space, get_type(var->GetType()), var->GetName());
			else
				fprintf(output, "\n%spublic %s %s = %s;", space, get_type(var->GetType()), var->GetName(), val);
		}
	}
	return 1;
}

int print_proc_update_clocks(IfList<IfVariable> *variables){
	int n = variables->GetCount();

	fprintf(output, "\n\n	public void updateClock(int val){");

	for (int i=0; i<n; i++){
		IfVariable *var = variables->GetAt(i);
		if (strcmp(var->GetType()->GetName(), "clock") == 0){
			fprintf(output, "\n		%s = val;", var->GetName());
		}
	}
	fprintf(output, "\n	}");
	return 1;
}

int print_procedure(IfProcedure *proc){
	if (proc == NULL)
		return 0;
	fprintf(output, "\n	//procedure\n	public %s %s(", get_type(proc->GetReturn()), proc->GetName());
	IfList<IfVariable> *vars = proc->GetParameters();
	int n = vars->GetSize();
	for (int i=0; i<n; i++){
		IfVariable *v = vars->GetAt(i);
		if (v == NULL)
			continue;
		if (i==0)
			fprintf(output, "%s %s", get_type(v->GetType()), v->GetName());
		else
			fprintf(output, ", %s %s", get_type(v->GetType()), v->GetName());
	}
	char *str = replaceAll(proc->GetBody(), "{#", "");
	str = replaceAll(str, "#}", "");
	fprintf(output, "){\n		%s\n	}", str);
	return 1;
}

int print_proc(const char * sysName, IfProcessEntity *proc){
	tranID = 0;
	const char *procName = proc->GetName();

	int len = strlen(procName) + 5;
	char *str = new char[len];

	sprintf(str, "%s.java", procName);
	
	printf("\n  Write the process [%s] to file [%s]", procName, str);

	output = fopen(str, "w");

	if (output == NULL){
		printf("\n     ==> Error: cannot create file [%s]\n", str);
		exit(0);
	}


	fprintf(output, "public class %s extends %s{", procName, sysName);

	print_variables("	", proc->GetVariables());

	//update clock
	print_proc_update_clocks(proc->GetVariables());

	IfList<IfProcedure> *p = proc->GetProcedures();
	int n = p->GetSize();
	for (int i=0; i<n; i++)
		print_procedure(p->GetAt(i));

	IfList<IfState> *states = proc->GetStates();
	n = states->GetCount();
	for (int i=0; i<n; i++){
		print_state(states->GetAt(i));
	}

	//print other
	fprintf(output, "\n	public void callState(String stateName){");
	for (int i=0; i<n; i++){
		IfState *s = states->GetAt(i);
		fprintf(output, "\n		if (stateName.equals(\"%s\")) state_%s();", s->GetName(), s->GetName());
	}
	fprintf(output, "\n	}");

	fprintf(output, "\n}");
	printf("\n      ==>   OK");
	return 1;
}

void print_constants(IfList<IfConstant> *constants){
	if (constants == NULL)
		return;

	int n = constants->GetCount();
	for (int i=0; i<n; i++){
		IfConstant *con = constants->GetAt(i);
		if (con->IsAction()){

		}else if (con->IsBoolean()){
			IfBooleanConstant *c = (IfBooleanConstant *) con;
			fprintf(output, "\n	public final boolean %s = %s;", c->GetName(),
					(c->GetValue() == IfBooleanConstant::TRUE)? "true" : "false");
		}else if (con->IsInteger()){
			IfIntegerConstant *c = (IfIntegerConstant *) con;
			fprintf(output, "\n	public final int %s = %d;", c->GetName(), c->GetValue());
		}else if (con->IsReal()){
			IfRealConstant *c = (IfRealConstant *) con;
			fprintf(output, "\n	public final float %s = %f;", c->GetName(), c->GetValue());
		}
	}
}

void print_type_definitions(IfList<IfType> *types){
	if (types == NULL)
		return;

	int n = types->GetCount();
	for (int i=0; i<n; i++){
		IfType *ty = types->GetAt(i);
		//enum
		if (ty->IsEnum()){
			IfEnumType *t = (IfEnumType *) ty;
			IfList<IfEnumConstant> *lits = t->GetLiterals();
			if (lits != NULL){
				fprintf(output, "\n	public static enum %s {\n		", t->GetName());
				int m = lits->GetSize();
				bool is_first = true;
				for (int j=0; j<m; j++)
					if (lits->GetAt(j) != NULL)
						if (is_first){
							fprintf(output, "%s", lits->GetAt(j)->GetName());
							is_first = false;
						}else
							fprintf(output, ", %s", lits->GetAt(j)->GetName());
				fprintf(output, "}");
			}
		}
		else if (ty->IsRange()){
			IfRangeType *rang = (IfRangeType *) ty;
			range_types[range_type_number ++ ] = strdup(rang->GetName());
		}
		else if (ty->IsRecord()){
			IfRecordType *rec = (IfRecordType *) ty;
			fprintf(output, "\n	public static class %s{", rec->GetName());
			print_variables("		",rec->GetFields());
			fprintf(output, "\n	}");
		}
	}
}

int print_system_define(IfSystemEntity *sys){
	const char *procName = sys->GetName();
	char *str = new char[strlen(procName) + 5];
	sprintf(str, "%s.java", procName);

	printf("\n  Write definitions of system [%s] to file [%s]", procName, str);
	output = fopen(str, "w");

	fprintf(output, "public class %s{", procName);
	print_constants(sys->GetConstants());
	print_variables("	", sys->GetVariables());
	print_type_definitions(sys->GetTypes());

	//class Clock
	/*fprintf(output, "\n	public class Clock extends Integer{\n		%s\n		 %s\n	}",
			"public void set(int val){\n			this = val;\n		}",
			"public void reset(){\n			this = 0;\n		}");
	*/

	fprintf(output, "\n	public void nextState(String stateName){}");

	fprintf(output, "\n	public void input(String className, Object var){}");

	fprintf(output, "\n	public void output(String className, Object var){}");

	fprintf(output, "\n}");
	printf("\n      ==>   OK");
	return 1;
}

/**
* truck technique
* I get an error when standard types are not declared. I don't understand why
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
		str.replace(d, 1, ";type integer = range 0..1;type pid = range 1..10;type boolean = range 0..1;type clock = range 0..1;");
	}
	FILE *des = tmpfile();
	fwrite(str.c_str(), sizeof(char), str.length(),des);
	fflush(des);
	rewind(des);
	return des;
}



int main(int argc, char * argv[]) {
  
  if (argc != 2) {
    fprintf(stderr, "\n Usage: %s ifFileName",argv[0]);
	 fprintf(stderr, "\n\n");
    return 0;
  }
  
  FILE *file;
  file = fopen(argv[1], "r");
  if (file == NULL){
	  fprintf(stderr, "Cannot open file: %s", argv[1]);
	  return 0;
  }

  output_folder = argv[1];
  //remove .if at the end of fileName
  output_folder[strlen(output_folder) - 3] = '\0';


  file = copy(file);
 

  IfSystemEntity *system = Load(file, NULL, 0);
  print_system_define(system);

  IfList<IfProcessEntity> *proc = system->GetProcesses();
  sysName = system->GetName();
  int n = proc->GetCount();
  for (int i=0; i<n; i++){
	  print_proc(sysName, proc->GetAt(i));
  }
  
  fclose(file);
  printf("\n\n");
  return 1; 
}


