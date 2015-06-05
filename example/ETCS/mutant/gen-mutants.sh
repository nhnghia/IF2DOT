#!/bin/sh

#MAJOR_HOME="../major"

cd ./simulator

LOG="../gen-mutants.log"

#echo "" > $LOG

echo "Clear contents in the folder: ./mutants" | tee $LOG

rm -R ../mutants/*

echo "Generate Mutants" >> $LOG

for com in OBU RBC
do
	echo ====$com========================== >> $LOG
	echo %$com
	for i in {1..6}
	do
		echo "- Compiling MML definition file of Error Type: $i" >> $LOG
	 	$MAJOR_HOME/bin/mmlc config.$i.$com.mml #&> /dev/null

		#echo "- Running Major with mutation"
		$MAJOR_HOME/bin/javac -J-Dmajor.export.mutants=true -J-Dmajor.export.directory=../mutants/Type$i/$com/ -XMutator=config.$i.$com.mml.bin ETCS.java Simulator.java &> /dev/null
		
		find ../mutants/Type$i/$com/* -maxdepth 0 -type d | wc -l  >>  $LOG
                echo `tail -1 $LOG`,%
	done
done

rm config.*.mml.bin
rm ETCS*.class Simulator*.class
rm mutants.log

cd ../
