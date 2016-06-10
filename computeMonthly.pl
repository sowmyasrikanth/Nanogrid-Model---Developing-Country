#!/usr/bin/perl

system ('rm test.txt');
$init = 1.0;

for($fn=1; $fn <=30; $fn = $fn + 1)
 {

open (INFILE, "$fn.txt");
open (OUTFILE, '>input.txt');
while ($line1=<INFILE>)
{
        print OUTFILE "$line1";
}
 close (INFILE);
 close (OUTFILE);

 system ('gcc -o nano nanogrid.c');
 system ("nano 151200 $init 1 0 1 0.0001 > out");
 
open (INFILE, 'out');
open (OUTFILE, '>>test.txt');
 
 while ($line1=<INFILE>)
{
        print OUTFILE "$line1";
}

 close (INFILE);
 close (OUTFILE);
 
 open (INFILE, 'out');
 open (OUTFILE, '>Step1.txt');

  while ($line1=<INFILE>)
{
        print OUTFILE "$line1";
}

 close (INFILE);
  close (OUTFILE);

 open (INFILE, 'Step1.txt');
while ($line1=<INFILE>)
{
	@comp1 = split(',',$line1);
	$init = @comp1[5];
  print "$init\n";
}
 close (INFILE);



}

