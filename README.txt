Project: Nanogrid Simulation - Developing Country

Author: Sowmya Srikanth, sowmyas@mail.usf.edu

Date: June 5th, 2016

Description: This project develops and controls the simulation model of a nanogrid deployed in a developing country.  

Instructions:

To run the model for a day:
1. Change the name of the input file to the day that you wish to run the model for.
   The data for the same can be found in the 24 hour data folder.
   
   Eg. fp = fopen("sunny.txt", "r");
   
2. Compile and run the model from the command line using the below commands:
   To compile : gcc -o nano nanogrid.can
   To run     : nano "size of the battery in Wm" "initial battery level" "curtail threshold 1" "curtail threshold 2" "curtail threshold 3" "price increment" > "output text file"
                
                Eg : for a battery of size 3250 kWh, with a 80% initial battery level, 100%, 0% and 100% as curtail thresholds 1,2 and 3 and a price increment of 1 microcent:
                nano 151200 0.80 1 0 1 0.0001 > output.txt
                
3. You can see the results in the output.txt file

To run the model for a month:
1. Place the nanogrid.c file in the folder based on the month the model is run for.
   Open the perl command line and type the command.
   perl computeMonthly.pl
   
2. The results can be viewed either in the text file or the csv file - depending on what is specified in the perl file
   Eg : if open (OUTFILE, '>>test6.txt');
        then, the output is in the test6.txt file