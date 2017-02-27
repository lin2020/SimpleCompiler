#!/bin/bash
echo -e "\nflex lexical.l"
flex lexical.l
if [ $? -eq 0 ]; 
then
		echo -e "\nbison -d syntax.y"
		bison -d syntax.y
		if [ $? -eq 0 ]; 
		then
			echo -e "\ngcc syntax.tab.c -lfl -ly -o parser"
			gcc syntax.tab.c -lfl -ly -o parser
			if [ $? -eq 0 ]; 
			then
				# echo -e "\n./parser exp4.1.c"
				# ./parser exp4.1.c
				echo -e "\n./parser exp4.2.c"
				./parser exp4.2.c
			else
				echo -e "\nerror"
			fi
		else
			echo -e "\nerror"
		fi
else
		echo -e "\nerror"
fi