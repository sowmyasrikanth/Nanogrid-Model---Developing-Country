//----- Include files -------------------------------------------------------
#include <stdio.h>                 // Needed for printf()

//----- Defines -------------------------------------------------------------
#define MAX                 100000        // Maximum number of ticks
#define OUTPUT_STATS         0            // Output statistics flag
#define OUTPUT_TEXT_SUMMARY  0            // Output in text file summary flag
#define OUTPUT_PARAMETERS    0            // Output parameters flag
#define OUTPUT_LOAD          0            // Output load flag
#define CONTROL_MECH         1            // Turn on the control mechanism
#define OUTPUT_EXCEL_SUMMARY 1            // Output data in excel
#define OUTPUT_STATS_1       0            // Output in excel file summary flag

//----- Function prototypes -------------------------------------------------
//Battery controller model
double battery(double availBattery, double localPrice, double priceIncrement);            
// High priority Load device model
double hpLoadDevice();                                    
// Low priority Load device model
double lpLoadDevice(double lBuyPrice, double localPrice, double LFPD); 

//------ Global Variables----------------------------------------------------
int ncDemandFlag;           // Keeps track of the power consumption mode of the device
int ncLoadCounter;       // Counter for the non curtailable load
double curtailThresh1;      // The curtail threshold for the first 8 hours (12 am - 8 am)
double curtailThresh2;      // The curtail threshold for the next 10 hours (8 am - 6 pm)
double curtailThresh3;      // The curtail threshold for the final 6 hours (6 pm - 12 am) 
double maxPrice;            // Maximum value of the local power price
double maxBattery;          // Maximum capacity of the battery [J]
double availBattery;        // Available energy from battery
double lowBattery;          // Threshold for low battery indication
double localPrice;          // Local power price of the nanogrid
double lBuyPrice;           // Price the low priority load is willing to pay
double hBuyPrice;           // Price the high priority load is willing to pay
double maxLBuyPrice;        // Maximum value of the low priority load buy price
double HFPD;                // Full power demand from a high priority load
double HFPD1;               // Full power demand from a high priority load - maximum mode
double HFPD2;               // Full power demand from a high priority load - minimum mode
int lpcounter;

//===========================================================================
//=  Main program                                                           =
//===========================================================================
int main(int argc, char *argv[])
{
  double availSource[MAX];    // Available power from source
  double LRPD;                // Reduced power demand from low priority load
  double LFPD;                // Full power demand from a low priority load
  double HSPD;                // The satisfied power demand of the high priority load
  double LSPD;                // The satisfied power demand of the low priority load
  double excess;              // The excess supply
  double avgLocalPrice;       // Average local price 
  double totalLPDemand;       // Total demand from low priority loads
  double totalHPDemand;       // Total demand from high priority loads
  double totalLPUnmet;        // Total unmet demand from low priority loads
  double totalHPUnmet;        // Total unmet demand from hugh priority loads 
  double totalExcess;         // Total excess demand
  double totalSource;         // Total Source 
  double remainingEnergy;     // Remaning energy after power supplied to load
  int    numTicks;            // Number of ticks of source energy
  int    i;                   // Tick number
  int    cutOff;              // Flag to cut off the curtailable load
  double totalEnergy;	        // The total energy available in the system-battery + source
  double priceIncrement;      // The local power price increment for that time interval
  int    timer;               // Timer to keep track of the time intervals to send the message
  double localPriceOld;       // Keeps track of the sell price in the prev time interval to check for a change in the sell price
  int    messageCount;        // The number of messages sent by the controller to the loads every 15 mins
  int    totalMessageCount;   // The total number of messages
     
  
  FILE *fp;

  fp = fopen("input.txt", "r");
  
  if(fp != NULL)
  {
    while(!feof(fp)) 
    {
        fscanf(fp, "%lf", &availSource[numTicks]);
        numTicks++;
    }
  }
    
  if ( argc < 2 ) /* argc should be 2 for correct execution */
  {
       /* We print argv[0] assuming it is the program name */
       printf( "usage: %s filename", argv[0] );
  }
  
  // Initialize battery level, low priority load thresholds and the local price increment
  sscanf(argv[1],"%lf",&maxBattery);
  sscanf(argv[2],"%lf",&lowBattery);
  availBattery = maxBattery * lowBattery;
  sscanf(argv[3],"%lf",&curtailThresh1);
  sscanf(argv[4],"%lf",&curtailThresh2);
  sscanf(argv[5],"%lf",&curtailThresh3);
  sscanf(argv[6],"%lf",&priceIncrement);
  
  // Intialize key variables to zero
  excess = LSPD = 0.0;
  cutOff = 0;
  avgLocalPrice = totalLPDemand = totalHPDemand = totalLPUnmet = totalHPUnmet = totalExcess = totalSource = 0.0;
  messageCount = timer = totalMessageCount = 0;
  HFPD1 = 55.0;
  HFPD2 = 0.0;
  HSPD = 0.0;
  LFPD = 62.0;
  HFPD = HFPD1;
  ncDemandFlag = 1;
  ncLoadCounter = 0;
  lpcounter = 0;
  
  // Initialize all price values
  localPrice = localPriceOld = 0.001;           //10 microcents
  lBuyPrice = 0.001 ;                           //10 microcents
  hBuyPrice = 0.001;                            //10 microcents initially
  maxPrice = (LFPD/(0.25*LFPD))*lBuyPrice;      //so, 40 cents

  // Output simulation parameters
  if(OUTPUT_PARAMETERS == 1)
  {
    printf("------------------------------------------------------------------------------------ \n");
    printf("-- Maximum Battery Capacity      			       = %lf\n", maxBattery);
    printf("-- Threshold of the battery  			           = %lf\n", lowBattery * 100);
    printf("-- Local Price                               = %f\n", localPrice);
    printf("-- Low priority load buyPrice                = %f\n", lBuyPrice);
    printf("-- High priority load buyPrice               = %f\n", hBuyPrice);
    printf("-- Maximum local price                       = %f\n", maxPrice);
    printf("-- High priority load demand                 = %f\n", HFPD);
    printf("-- Low priority load demand                  = %lf\n", LRPD);
  }
  
  if (OUTPUT_STATS == 1)
  {
    //printf("------------------------------------------------------------------------------------ \n");
    printf("Load,Minute,Source,Battery,Local Price,FullDemand,ReducedDemand,Excess,Unmet\n"); 
  }
  
  if (OUTPUT_STATS_1 == 1)
  {
    //printf("------------------------------------------------------------------------------------ \n");
     printf("NCDemand,LPDemand,HPUnmet,LPUnmet,excess,battery,source,messagecount\n");
  }
  
  // Main simulation loop
  for (i=0; i<numTicks; i++)
  { 
    timer++;
    
    totalLPUnmet = totalLPUnmet + (LRPD - LSPD);
    totalHPUnmet = totalHPUnmet + (HFPD - HSPD);
    totalExcess = totalExcess + excess;
    totalLPDemand = totalLPDemand + LRPD;
    totalHPDemand = totalHPDemand + HFPD;
    totalSource = totalSource + availSource[i];
   
    //If there is a change in the local price value, controller sends a message   
    if(localPriceOld != localPrice)
    {
      messageCount++;
    }    
    
    //Count the number of messages per hour
    if((timer%60) == 0)
    {
      totalMessageCount = totalMessageCount + messageCount;
      messageCount = 0;
    }
    
    localPriceOld = localPrice;
        
    HFPD = hpLoadDevice();
    remainingEnergy = availSource[i] - HFPD;
    
    if(availBattery < HFPD)
      HSPD = availBattery;
    else
      HSPD = HFPD;
    
    // Output key statistics for start of this tick
    if(OUTPUT_LOAD == 1)
    {
       printf("Non Curtailable load1,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
          i, availSource[i], availBattery/maxBattery, localPrice, HFPD, HFPD, excess, HFPD-HSPD);
    }

    LRPD = lpLoadDevice(lBuyPrice,localPrice,LFPD);
    remainingEnergy = remainingEnergy - LRPD;
   
    if(availBattery < LRPD)
      LSPD = availBattery;
    else
      LSPD = LRPD;
    
    if(OUTPUT_LOAD == 1)
    {    
      printf("Curtailable loads,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
        i, availSource[i], availBattery/maxBattery, localPrice, LFPD, LRPD, excess, LRPD - LSPD);
    }
    
    availBattery = availBattery + remainingEnergy;
    
    if (availBattery > maxBattery)
    {
      excess = availBattery - maxBattery;
      availBattery = maxBattery;
    }
    else if (availBattery < 0.0)
    {
      availBattery = 0.0;
    }
    else
      excess = 0.0;
      
    if(CONTROL_MECH == 1)
    { 
      localPrice = battery(availBattery,localPrice,priceIncrement);
    }
    
    //Set the minimum and maximum local price values
    if (localPrice < 0.0001)
    {
      localPrice = 0.0001;
    }
    else if (localPrice > maxPrice)
    {
      localPrice = maxPrice;
    }
  }
  
  if (OUTPUT_TEXT_SUMMARY == 1)
  {
    printf("---------------------------------------------------------------\n");
    printf("-- Total non curtailable demand (energy)        = %f  \n", totalHPDemand);
    printf("-- Total curtailable demand (energy)            = %f  \n", totalLPDemand);
    printf("-- Total Curtailable unmet demand (energy)      = %f  \n", totalHPUnmet);
    printf("-- Total Non Curtailable unmet demand (energy)  = %f  \n", totalLPUnmet);
    printf("-- Total excess supply (energy)                 = %f  \n", totalExcess);
    printf("-- Percentage of battery remaining              = %f  \n", availBattery/maxBattery);
    printf("-- Flag                                         = %f  \n", availBattery/maxBattery);
    printf("-- Total Source Energy                          = %f  \n", totalSource);
    printf("-- Total Number of Messages sent                = %d  \n", totalMessageCount);
    printf("---------------------------------------------------------------\n");
  }
  if(OUTPUT_EXCEL_SUMMARY == 1)
  {  
    printf("%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d\n", totalHPDemand, totalLPDemand, totalHPUnmet, totalLPUnmet, totalExcess, availBattery/maxBattery, totalSource, totalMessageCount );
  }

  return(0);
}
//===========================================================================
//=  Function to model a battery controller                                 =
//=-------------------------------------------------------------------------=
//=  Inputs: availBattery, sellPrice, sellIncrement                         =
//=  Returns: sellPrice                                                     =
//===========================================================================
double battery(double availBattery, double localPrice, double priceIncrement)
{
  if(availBattery < (0.20 * maxBattery))
  {
    localPrice = localPrice + priceIncrement;
  }
  else if(availBattery > (0.80 * maxBattery))
  {
    localPrice = localPrice - priceIncrement;
  }
  if(localPrice > maxPrice)
    localPrice = maxPrice;
  else if(localPrice < 0.0001)
    localPrice = 0.0001;
  
  return localPrice;
}

//===========================================================================
//=  Function to model a curtailable load device                            =
//=-------------------------------------------------------------------------=
//=  Inputs: buyPrice, sellPrice                                            =
//=  Returns: demand                                                        =
//===========================================================================
double lpLoadDevice(double lBuyPrice, double localPrice, double LFPD)
{
  double demand;    // Energy demand from load
  double fDemand;   
   
  // Rule to determine how much the device can/will pay for energy
  if(localPrice < 0.001)
    lBuyPrice = localPrice;
  else
    lBuyPrice = 0.001;
  
  //Determine Max Demand for the curtailable load based on time of day - Has to be done by the load
  if (lpcounter >=0 && lpcounter < 481)
  {
    fDemand = 1*LFPD;
  }
  else if (lpcounter >= 481 && lpcounter < 1080)
  {
    fDemand = 0*LFPD;
  }
  else if (lpcounter >=1080)
  {
    fDemand = 1*LFPD;
  }
  
  // Rule for determining demand as function of buyPrice and sellPrice
  demand = (lBuyPrice / localPrice) * fDemand;

  // Cannot demand more than LOAD_MAX in any case
  if (demand > fDemand)
    demand = fDemand;
  
  lpcounter++;
  return(demand);
}


//===========================================================================
//=  Function to model a non curtailable load device                        =
//=-------------------------------------------------------------------------=
//=  Inputs: buyPrice, sellPrice                                            =
//=  Returns: The revised buying price                                      =
//===========================================================================

double hpLoadDevice()
{
  int i, hDemand;
  
   if(ncLoadCounter <= 20 && ncDemandFlag == 1)
   {
     hDemand = HFPD1;
     if(ncLoadCounter == 20)
     {
      ncLoadCounter = 0;
      ncDemandFlag = 0;
     }
    }
    else if(ncLoadCounter <= 20 && ncDemandFlag == 0)
    {
      hDemand = HFPD2;
      if(ncLoadCounter == 20)
      {
        ncLoadCounter = 0;
        ncDemandFlag = 1;
      }
    }
    ncLoadCounter++;
    
  return(hDemand);
}

