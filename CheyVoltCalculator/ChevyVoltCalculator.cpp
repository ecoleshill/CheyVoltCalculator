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
	RunCalcs();

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
				string tmp = ReadLine.substr(0, Loc);
				eRate.StartTime = Get24HrTime(tmp);

				int Loc2 = ReadLine.find(',');
				tmp = ReadLine.substr(Loc+1, (Loc2 - Loc)-1);
				eRate.EndTime = Get24HrTime(tmp);
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
				extrRates.Total = 0;		//init the running total for this extra rate to zero
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
			if (result == -1)
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

//This function performs all the calculations w.r.t. the data and rates loaded from the file
//Output:  ASCII txt file containing the results
void RunCalcs()
{
	string filename;							//The output filename
	double RateToUse;
	double TotalElec = 0;						//Total amount of electricity used
	double TotalElecCost = 0;					//Total sum of electricity cost based on rate used

	//determine the current date and time so a filename can be generated
	time_t rawTime;
	struct tm* timeInfo = new tm();

	time(&rawTime);
	localtime_s(timeInfo, &rawTime);

	filename = to_string(timeInfo->tm_year + 1900) + to_string(timeInfo->tm_mon + 1) + to_string(timeInfo->tm_mday) + ".txt";
	
	ofstream ofs(filename, ofstream::out);

	//Setup the header
	ofs << "Charge Start, Charge Result, KwHr, Rate, Cost of Elec, ";
	for (size_t i = 0; i < MyDelivery.size(); i++)
		ofs << MyDelivery[i].Name << ",";
	
	ofs << std::endl;

	//Loop through the downloaded OnStar history and perform the calculations
	for (size_t x = 0; x < MyHistory.size(); x++)
	{
		//Check to see if a weekend
		if ((MyHistory[x].DateTime.tm_wday == 0) || (MyHistory[x].DateTime.tm_wday == 6))
		{
			RateToUse = GetRate("2300");			//use evening rate for weekend
		}
		else
		{
			string timeStr = to_string(MyHistory[x].DateTime.tm_hour);

			//12 noon is 00 hrs - need to handle this as special case
			if (timeStr == "0")
				timeStr = "12";

			string min = to_string(MyHistory[x].DateTime.tm_min);
			
			//Need to make sure the minutes are always at least 2 characters wide
			if (min.length() == 1)
				min = "0" + min;

			timeStr += min;

			//Special case to handle a plugin with a delayed charge to 7pm for the lower rate
			//If a partial charge, charge a correct rate, if a full charge push time to 1900hrs
			int TheTime = atoi(timeStr.c_str());
			if ((TheTime > 1600) && (TheTime < 1900) && (MyHistory[x].ChargeResult == "Full"))
				timeStr = "1900";

			RateToUse = GetRate(timeStr);
		}
		
		//Start writing the output file for this record
		char buffer[80];
		strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &MyHistory[x].DateTime);
		ofs << buffer << ",";
		ofs << MyHistory[x].ChargeResult << ",";
		ofs << MyHistory[x].KwHr << ",";
		ofs << RateToUse << ",";

		//Calculate the cost of the electricity
		TotalElec += MyHistory[x].KwHr;
		TotalElecCost += MyHistory[x].KwHr * RateToUse;
		ofs << MyHistory[x].KwHr * RateToUse << ", ";

		//Loop through all the delivery rates and print them out.
		for (size_t i = 0; i < MyDelivery.size(); i++)
		{
			ofs << MyHistory[x].KwHr * MyDelivery[i].Rate << ",";
			MyDelivery[i].Total += MyHistory[x].KwHr * MyDelivery[i].Rate;
		}
		ofs << endl;
	}

	//Print out the totals
	ofs << "Total Electricity Use:  " << TotalElec << std::endl;
	ofs << "Total Electricity Cost: $" << TotalElecCost << std::endl;
	for (size_t i = 0; i < MyDelivery.size(); i++)
		ofs << MyDelivery[i].Name << " Total: " << MyDelivery[i].Total << std::endl;

	ofs.close();
}

//This function returns a clean string - removing all the extra characters from the read
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

//This function converts the a time string into a 24hr time int value
int Get24HrTime(string Time)
{
	int TheTime = 0;

	int index = Time.find('m');			//Time with be AM or PM, so find the 'M' as the common element
	if (index < 0)
		index = Time.find('M');
	
	index -= 1;							//The location of the AM/PM starting point

	string timeValue = Time.substr(0, Time.length() - (index-1));
	TheTime = atoi(Time.c_str());

	if ((Time[index] == 'P') || (Time[index] == 'p'))
		TheTime += 12;

	TheTime = TheTime * 100;

	return TheTime;
}

//This function returns the charge rate to apply /kwhr
//Input:  string containing HHMM
//Output: the kwhr rate for that time period
double GetRate(string Time)
{
	int index = 0;
	int TheTime = atoi(Time.c_str());

	for (size_t x = 0; x < MyeRates.size(); x++)
	{
		if (MyeRates[x].StartTime < MyeRates[x].EndTime)
		{
			if ((TheTime >= MyeRates[x].StartTime) && (TheTime < MyeRates[x].EndTime))
				return MyeRates[x].Rate;
		}
		else		//Need to handle the special case when the time span goes over midnight
		{
			if (((TheTime >= MyeRates[x].StartTime) && (TheTime < 2400)) || ((TheTime > 0) && (TheTime < MyeRates[x].EndTime)))
				return MyeRates[x].Rate;
		}
	}

	return index;
}