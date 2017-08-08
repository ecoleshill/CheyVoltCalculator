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
	string StartTime;			//Beginning of the rate period
	string EndTime;				//End of the rate period
	double Rate;				//The rate for the period
};


//System Variables
vector <DeliveryRates> MyDelivery;		//The vector space containing all the delivery hydro rates loaded from the configuration file
vector <ElectricityRates> MyeRates;		//The vector space containing all the hydro rate periods loaded from the configuration file

//Functional Logic
bool LoadCfg();					//This function loads the default configuration file

#endif