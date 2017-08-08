/*****************************************************************************************
Date:		August 7, 2017
Author:		Dr. James Elliott Coleshill
Purpose:	This file contains the functionality for the Chevy Volt Calculator.  Designed
			to input hydro rates and OnStar Charge History CSV files and output a detailed
			cost analysis for the car
*****************************************************************************************/
#include "ChevyVoltCalculator.h"

int main(char argc, char *argv)
{
	LoadCfg();
	return 1;
}

//Supporting Logic functions are found below

//This function loads the configuration file that contains all the hydro charging information
bool LoadCfg()
{
	string ReadLine;			//The current line read from the file


	ifstream ifs("HydroConfig.txt", ifstream::in);

	if (!ifs)
	{
		cerr << "ERROR:  Failed to load the HydroConfig.txt file" << endl;
		return false;
	}

	while (!ifs.eof())
	{
		getline(ifs, ReadLine);
		
		//Validate the input string and processing accordingly
		if (ReadLine == "#Electricity Rates")
		{
			ElectricityRates eRate;			//an Electricity rate structure to read the data into

			getline(ifs, ReadLine);
			while (ReadLine != "#end")
			{
				//parse the data and push it onto the MyeRates vector space
				int Loc = ReadLine.find('-');
				eRate.StartTime = ReadLine.substr(0, Loc);
				int Loc2 = ReadLine.find(',');
				eRate.EndTime = ReadLine.substr(Loc+1, (Loc2 - Loc)-1);
				eRate.Rate = stod(ReadLine.substr(Loc2 + 1, ReadLine.length() - Loc2));
				MyeRates.push_back(eRate);
				getline(ifs, ReadLine);
			}
		}
		else if (ReadLine == "#Other Rates")
		{
			DeliveryRates extrRates;

			getline(ifs, ReadLine);
			while (ReadLine != "#end")
			{
				//parse the data and push it onto the MyeRates vector space
				int Loc = ReadLine.find(',');
				extrRates.Name = ReadLine.substr(0, Loc);
				extrRates.Rate = stod(ReadLine.substr(Loc+1, ReadLine.length() - Loc));
				MyDelivery.push_back(extrRates);
				getline(ifs, ReadLine);
			}
		}
	}
	return true;
}