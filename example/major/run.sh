#!/bin/sh
if [ "$#" -ne 2 ]; then
	 echo "Using: ./run.sh config obj"
	 echo "  - config: 1, 2, 3, 4, 5, 6"
	 echo "  - obj   : OBU, RBC"
	 echo "Example: ./run.sh 1 OBU"
	 exit
fi


#clear screen
clear
clear

MAJOR_HOME="../../"

echo "- Compiling MML definition file"
$MAJOR_HOME/bin/mmlc config.$1.$2.mml

echo "- Running Major without mutation"
$MAJOR_HOME/bin/javac Main.java ETCS.java Simulator.java

echo "- Running Major with mutation"
$MAJOR_HOME/bin/javac  -XMutator=config.$1.$2.mml.bin Main.java ETCS.java Simulator.java

echo "- Compiling test case"
$MAJOR_HOME/bin/javac  -Xlint:unchecked -cp .:$MAJOR_HOME/config/config.jar Test$2.java

echo "- Executing test case"
echo
java  -cp .:$MAJOR_HOME/config/config.jar Test$2 > result.$1.$2.txt

tail -3 result.$1.$2.txt

mv mutants.log mutants.$1.$2.log.txt
rm *.class
rm *.mml.bin