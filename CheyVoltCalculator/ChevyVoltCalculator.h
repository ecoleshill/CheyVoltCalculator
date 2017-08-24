/*****************************************************************************************
Date:		August 7, 2017
Author:		Dr. James Elliott Coleshill
Purpose:	This file contains all the system variables and vector spaces for the Chevy
			Volt Calculator
*****************************************************************************************/

#ifndef _CHEVYVOLTCALCULATOR_H_
#define _CHEVYVOLTCALCULATOR_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
using namespace std;

//This structure defines all the "extra" charges for delivery of electricity
struct DeliveryRates
{
	string Name;				//The name of the hydro rate
	double Rate;				//The charge rate per kw/hr
};

//This structure defines all the different rate periods
struct ElectricityRates
{
	int StartTime;			//Beginning of the rate period
	int EndTime;				//End of the rate period
	double Rate;				//The rate for the period
};

//This structure defines the telemetry from the OnStar System (ChargeHistory)
struct ChargeHistory
{
	tm DateTime;				//Charge start date and time
	string ChargeResult;		//Determines if charge was a full/partial charge
	double KwHr;				//Number of Kw/hrs of electricity used
};

//System Variables
vector <DeliveryRates> MyDelivery;		//The vector space containing all the delivery hydro rates loaded from the configuration file
vector <ElectricityRates> MyeRates;		//The vector space containing all the hydro rate periods loaded from the configuration file
vector <ChargeHistory> MyHistory;		//The vector space containing all the charge history from OnStar

//Functional Logic
bool LoadCfg();							//This function loads the default configuration file
bool LoadTelem(string filename);			//This function loads the telemetry file into memory

void RunCalcs();						//This function runs the volt calculations and writes the results to the output data file

//Supporting Functions
string ClearString(string strLine);		//This function removes all \ and " characters from the string
int Get24HrTime(string Time);			//This function converts the string to a 24hr time integer
double GetRate(string Time);				//This function returns the rate for a specific time

#endif