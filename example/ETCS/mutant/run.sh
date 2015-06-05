#!/bin/sh
#MAJOR_HOME="./major"
if [ "$#" -ne 1 ]; then
	 echo "Using: ./run-all.sh tc_folder"
	 exit
fi

rm *.java &> /dev/null
rm result.*.txt &> /dev/null
rm *.class &> /dev/null

echo "" > result.txt

echo copy test cases from $1

cp $1/*OBU*.tc obu.testcase.tc
cp $1/*RBC*.tc rbc.testcase.tc

for com in OBU RBC
do
	echo ===$com >> result.txt
    for i in {1..6}
    do
		echo ---TYPE $i >> result.txt

		MUTANT=""
		for mut in ./mutants/Type$i/$com/*; do
		    if [ -d "${mut}" ]; then
#		        echo $mut
				#copy original Java files of the simulator
				cp ./simulator/*.java ./
				
				#copy mutation files
				cp $mut/*.java ./
				
				javac Main.java ETCS.java Simulator.java &> /dev/null
				
				echo >> result.$com.txt
				echo Testing Mutant $mut >> result.$com.txt
				
				#echo "- Executing test case"
				java Main $com >> result.$com.txt
				
				VERDICT=`tail -1 result.$com.txt`
				
				echo Mutant $mut, $VERDICT >> result.txt
				
				if [ "$VERDICT" == "verdict: false" ]; then
					MUTANT=`basename $mut`,$MUTANT
				fi
		    fi
		done
		
		#remove the last , in $MUTANT, e.g., "{1,2,},%" --> "{1,2},%"
    	echo "{$MUTANT},%" | sed -e 's/,},%$/},%/'
    done
	
	
done

mv result*.txt $1/
rm *.testcase.tc
rm *.class
rm ETCS.java Simulator.java Main.java
