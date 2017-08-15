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

	LoadTelem("chargingHistory.txt");
	return 1;
}

//Supporting Logic functions are found below

//This function loads the configuration file that contains all the hydro charging information
bool LoadCfg()
{
	string ReadLine;			//The current line read from the file


	ifstream ifs("HydroConfig.txt", ifstream::in);

	if (!ifs.failbit)
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
	ifs.clear();
	ifs.close();
	return true;
}

//This function loads the ChargeHistory telemetry file from OnStar into a vector space
bool LoadTelem(string filename)
{
	string ReadLine;			//The current line from the file
	ChargeHistory NewCharge;	//The current charge history parsed into a structure
	int Location;				//The search location results from string parsing

	ifstream ifs(filename, ifstream::in);

	if (!ifs.failbit)
	{
		cerr << "ERROR:  Failed to load " << filename << " file" << endl;
		return false;
	}

	getline(ifs, ReadLine);			//Read an ignore the first line of the file - we don't need the header information
	while (!ifs.eof())
	{
		getline(ifs, ReadLine);

		ReadLine = ClearString(ReadLine);

		//Special case - catch last line and ignore it
		Location = ReadLine.find("Total");
		if ((Location == -1) && (ReadLine.length() > 0))
		{
			Location = ReadLine.find(",");
			string tmp = ReadLine.substr(0, Location);

			//Parse out the date information
			int result = tmp.find('/');
			NewCharge.DateTime.tm_mon = atoi(tmp.substr(0, result).c_str()) - 1;
			tmp = tmp.substr(result + 1, tmp.length() - result);
			result = tmp.find('/');
			NewCharge.DateTime.tm_mday = atoi(tmp.substr(0, result).c_str());
			tmp = tmp.substr(result + 1, tmp.length() - result);
			result = tmp.find(' ');
			NewCharge.DateTime.tm_year = atoi(tmp.substr(0, result).c_str()) - 1900;

			//Parse out the time information

			tmp = tmp.substr(8, tmp.length() - 9);
			int hr = atoi(tmp.substr(0, 2).c_str());
			int min = atoi(tmp.substr(3, 2).c_str());

			if (tmp[6] == 'P')
				hr += 12;

			NewCharge.DateTime.tm_hour = hr;
			NewCharge.DateTime.tm_min = min;
			NewCharge.DateTime.tm_sec = 0;

			mktime(&NewCharge.DateTime);

			result = ReadLine.find("Full");
			if (result == 0)
			{
				result = ReadLine.find("Partial");
				NewCharge.ChargeResult = "Partial";
			}
			else
				NewCharge.ChargeResult = "Full";

			result = ReadLine.find_last_of(',');
			tmp = ReadLine.substr(result + 1, ReadLine.length() - result);
			NewCharge.KwHr = atof(tmp.c_str());

			MyHistory.push_back(NewCharge);
		}
	}
	ifs.close();
	return true;
}

string ClearString(string strLine)
{
	string newString;
	
	for (size_t x = 0; x < strLine.length(); x++)
	{
		if ((strLine[x] != '\\') && (strLine[x] != '"'))
			newString += strLine[x];
	}

	return newString;
}