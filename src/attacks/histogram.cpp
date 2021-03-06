#include "main.h"

/* example constructor, sets some options */
Histogram::Histogram()
{
    // declare options, keep options as uppercase
    vector<string> temp;
    temp.push_back("INPUTFILE");
    temp.push_back("OUTPUTFILE");
	temp.push_back("BLOCKSIZE");
	temp.push_back("CHARSETSIZE");
	temp.push_back("SORTED");
	temp.push_back("SHOWFREQUENCY");
    set_opts(temp);

    // set default values, option must exist or error will printed
    set_opt_value("OUTPUTFILE", "histogram");
	set_opt_value("BLOCKSIZE", "1");
	set_opt_value("CHARSETSIZE", "256");
	set_opt_value("SORTED", "0");
	set_opt_value("SHOWFREQUENCY", "0");
}

/* helper function that doesn't really do anything */
void Histogram::test()
{
    cout << "example test" << endl;
}

/* I am overriding the default module function
 * for displaying the description text
 */
void Histogram::disp_desc()
{
	cout << "Module: attacks/histogram\n\tThis module aids in the generation of character distribution histograms.\n\tOutput is generated in a CSV format of \"VALUE,OCCURENCES;\"\n\tThe BLOCKSIZE option is used to generate a set of histograms, one for each character in a block.\n\tThe CHARSETSIZE option is used to set the maximum range of the histogram(s).\n\t" << endl;
    disp_opts();
    cout << endl;
}

int Histogram::run()
{
    // perform error checking on options first
    if (options["INPUTFILE"].empty()) {
        cout << "[-] Please specify an input file" << endl;
        return 1;
    }

    if (options["OUTPUTFILE"].empty()) {
        cout << "[-] Please specify an output file" << endl;
        return 2;
    }

	const int blockSize = stoi(options["BLOCKSIZE"]);
	const int charSetSize = stoi(options["CHARSETSIZE"]);

	if (blockSize < 1){
		cout << "[-] Block size should be greater than 0" << endl;
		return 3;
	}

	vector< vector<int> > charCounts;
	charCounts.resize(blockSize);
	for (unsigned int index = 0; index < charCounts.size(); index++){
		charCounts[index].resize(charSetSize);
	}

	ifstream in;
    ofstream out;
    string buff;

	if (process_input(&in, &charCounts, blockSize, charSetSize) != 0) {
		return 4;
	}

	vector<int> charTotal(blockSize), maxLength(blockSize), maxCount(blockSize);
	analyze_input(&charCounts, &charTotal, &maxLength, &maxCount);
	
	process_output(&out, &charCounts, &charTotal, &maxLength);

    cout << "[*] Closing files" << endl;
    in.close();
    out.close();

    return 0;
}

int Histogram::process_input(ifstream* in, vector< vector<int> >* charCounts, const int blockSize, const int charSetSize) {
	cout << "[*] Opening file: " << options["INPUTFILE"] << endl;
	in->open(options["INPUTFILE"]);


	cout << "[*] Processing input file..." << endl;
	char inChar;
	int blockIndex = 0, charIndex;
	while (!in->eof()) {
		in->get(inChar);
		charIndex = ((inChar < 0) ? (charSetSize + (int)inChar) : (int)inChar);
		if (charIndex < 0 || charIndex > charSetSize) {
			cout << "[-] Character set size option is smaller than actual size." << endl;
			return 4;
		}
		(*charCounts)[blockIndex][charIndex]++;
		blockIndex = (blockIndex + 1) % blockSize;
	}

	return 0;
}

int Histogram::analyze_input(vector< vector<int> >* charCounts, vector<int>* charTotal, vector<int>* maxLength, vector<int>* maxCount) {
	cout << "[*] Analyzing input..." << endl;

	for (unsigned int blockIndex = 0; blockIndex < charCounts->size(); blockIndex++) {
		for (unsigned int charIndex = 0; charIndex < (*charCounts)[blockIndex].size(); charIndex++) {
			int temp = (*charCounts)[blockIndex][charIndex];
			(*charTotal)[blockIndex] += temp;
			if (temp > (*maxCount)[blockIndex]) (*maxCount)[blockIndex] = temp;
		}

		int maxCtCopy = (*maxCount)[blockIndex];
		while (maxCtCopy /= 10) (*maxLength)[blockIndex]++;
	}

	return 0;
}

int Histogram::process_output(ofstream* out, vector< vector<int> >* charCounts, vector<int>* charTotal, vector<int>* maxLength) {
	vector<int> charIndexRef;
	charIndexRef.resize((*charCounts)[0].size());
	for (unsigned int charIndex = 0; charIndex < charIndexRef.size(); charIndex++) {
		charIndexRef[charIndex] = charIndex;
	}

	cout << "[*] Opening file: " << options["OUTPUTFILE"] << endl;
	out->open(options["OUTPUTFILE"]);

	cout << "[*] Writing output..." << endl;

	for (unsigned int blockIndex = 0; blockIndex < (*charCounts).size(); blockIndex++) {
		vector<int> charIndexTemp = charIndexRef;
		if (stoi(options["SORTED"]) != 0) {
			sort_arrays(&((*charCounts)[blockIndex]), &charIndexTemp);
		}

		(*out) << "Histogram for block index " << blockIndex << ": " << endl;
		
		for (unsigned int charIndex = 0; charIndex < (*charCounts)[blockIndex].size(); charIndex++) {
			if (stoi(options["SHOWFREQUENCY"]) == 0) {
				int numSpaces = 0, temp = (*charCounts)[blockIndex][charIndex];
				while (temp /= 10) numSpaces++;
				numSpaces = (*maxLength)[blockIndex] - numSpaces;
				(*out) << charIndexTemp[charIndex] << "," << string(numSpaces + 1, ' ') << (*charCounts)[blockIndex][charIndex] << "; \n";
			}
			else {
				float percent = ((float)(*charCounts)[blockIndex][charIndex] / (float)(*charTotal)[blockIndex]) * 100;
				(*out) << fixed << charIndexTemp[charIndex] << ", "<< percent << "; \n";
			}
		}
	}

	return 0;
}

int Histogram::sort_arrays(vector<int>* charCounts, vector<int>* charIndexArray) {
	for (unsigned int mainIndex = 0; mainIndex < (*charCounts).size(); mainIndex++) {
		int countValue = (*charCounts)[mainIndex],
			countIndex = (*charIndexArray)[mainIndex],
			swapIndex = mainIndex - 1;

		while (swapIndex >= 0 && (*charCounts)[swapIndex] < countValue) {
			(*charCounts)[swapIndex + 1] = (*charCounts)[swapIndex];
			(*charIndexArray)[swapIndex + 1] = (*charIndexArray)[swapIndex];
			swapIndex--;
		}

		(*charCounts)[swapIndex + 1] = countValue;
		(*charIndexArray)[swapIndex + 1] = countIndex;
	}

	return 0;
}
